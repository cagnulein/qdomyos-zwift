# DIRCON server lifetime — handoff and refactor plan

Written 2026-08-12, to continue the work elsewhere. Everything below was established
by debugging QZ and Rouvy running on the same Windows 11 machine over one session.
Branch point: `e6e1e34e` on `kind-of-stable`.

Amended after a re-read against the code: one claim about mid-session disconnects was
wrong and is corrected in "Why it matters", two line references were off, and the plan
gained the device-rebinding and port-derivation detail it was missing. The claims about
Rouvy's own behaviour (its DLL imports, its caching, the log timestamps) come from that
debugging session and were not re-verifiable from this repository.

The goal of that session was to get Rouvy to discover and connect to QZ's DIRCON
endpoint. It now does. What is left is the last structural problem, described in
"The refactor" below, which is the reason this branch exists.

## Scope

**Bikes only, desktop only.** The target is the FTMS test bike (`YPBM001264`) talking to
Rouvy on Windows. Treadmills, ellipticals and rowers are explicitly out of scope, and so
are the iOS-specific virtual-device paths. Where this document mentions
`virtualtreadmill` or an iOS branch it is for context, not as work to be done — narrowing
to one machine type is what makes the refactor small, and the reasoning behind that is in
"Shape of the change" step 3.

## What the session fixed (all merged on `kind-of-stable`)

Four independent faults, each on its own sufficient to break discovery completely.
They are listed because each one masked the next, and anyone re-testing needs to
know which are already closed.

| # | Fault | Commit |
|---|---|---|
| 1 | `localipaddress::getIP` returned a null address on every desktop platform, so the mDNS A record was published empty | `e390b331` |
| 2 | Windows `Dnscache` mDNS had been disabled, which is Rouvy's actual mDNS client | host config, not code |
| 3 | mDNS announcements left by whichever single interface the OS picked, normally a virtual adapter | `5d0986ad` |
| 4 | No goodbye on most exit paths, and the service was announced exactly once | `e6e1e34e` |

Detail on all four, plus the diagnostic commands, is in
[BUILDING-ON-WINDOWS.md](BUILDING-ON-WINDOWS.md) under "Rouvy on the same PC".

Two things worth knowing that are easy to get wrong:

- **Rouvy does not use Bonjour.** `Rouvy_Data/Plugins/x86_64/DnsZeroConfLib.dll`
  imports `DnsServiceBrowse` from Windows' own `DNSAPI.dll`. Bonjour's `mdnsNSP.dll`
  loads into the process only as a Winsock namespace provider and is not in the
  discovery path. Setting `Dnscache\Parameters\EnableMDNS = 0` makes the browse fail
  *and hard-crashes Rouvy* - its failure callback re-enters `StopBrowsing` on a
  half-built handle. Leave mDNS enabled.
- **QZ and Dnscache coexist on UDP 5353.** qmdnsengine binds with `ShareAddress`,
  falling back to `ReuseAddressHint`. Do not try to make QZ the sole owner of 5353.

## The problem this branch is for

QZ's DIRCON endpoint is **ephemeral**. Both the TCP listener and the mDNS
advertisement are created when a bike connects, and destroyed when it goes away:

```
virtualbike::virtualbike(...)                      // src/virtualdevices/virtualbike.cpp:48
    -> new DirconManager(Bike, ...)                // only if dircon_yes
        -> DirconProcessor::init()                 // src/devices/dircon/dirconprocessor.cpp
            -> initServer()                        // listen on 36866
            -> initAdvertising()                   // publish _wahoo-fitness-tnp._tcp
```

`virtualtreadmill` does the same at `src/virtualdevices/virtualtreadmill.cpp:26`, but
without the attach/detach machinery described below — out of scope, see "Scope".

The internal ordering is already correct - the log shows `Dircon TCP Server RV true`
before `Dircon Adv init`, so QZ never advertises a port it is not yet listening on.
The problem is the *lifetime*, not the ordering.

### Why it matters

Rouvy caches discovery results and **does not retry after a failed connect**. So any
window in which a client holds a record while the socket is absent produces a sticky
failure that survives until Rouvy's cache is cleared by hand. Observed exactly:

```
21:19:55  WftnpClient - Try to connect to ["192.168.0.107":36866]
21:19:57  WftnpClient - Failed to connect
```

QZ was running at 21:19:55 but had no bike yet, so nothing was listening. The bike
finished subscribing at 21:20:10, fifteen seconds too late. Rouvy re-discovered the
service repeatedly afterwards (`Creating device: [WFTNP, ELITE AVANTI 01234 W]` at
21:20:44, 21:21:32, 21:22:25) but never attempted another connection.

`e6e1e34e` narrows the window - a goodbye now invalidates the record on a clean quit -
but it cannot close it. A hard kill or a crash still leaves a record pointing at a dead
port, and the user-visible symptom is a trainer that Rouvy lists but refuses to pair
with.

A bike that drops mid-session is **not** one of those cases, contrary to what the first
draft of this document claimed. Neither of the two possible outcomes produces a live
record over a dead port:

- The device object survives — the normal path. Every
  `connect(..., SLOT(restart()))` in `src/devices/bluetooth.cpp` is commented out (105 of
  them), and the only live `restart()` call comes from the gym-mode device-name handler
  at `bluetooth.cpp:3786`. The listener stays up and the record stays valid; the client
  just sees stale values.
- The device object is destroyed, and `~ProviderPrivate()` calls `sayGoodbye()`
  (`provider.cpp:66-68`), which invalidates the record before the socket goes.

So the window that actually matters is the one at startup: QZ running, no bike connected
yet, nothing listening on 36866. That is the case in the log below, and it is what the
refactor closes.

### The workaround users need today

1. Start Rouvy, let it browse
2. Start QZ
3. **Connect the bike in QZ** — this is when the endpoint actually appears
4. Only then connect the trainer in Rouvy

## The refactor

Give the DIRCON endpoint the lifetime of the process rather than of the bike: listen
and advertise from startup when `dircon_yes` is set, and serve zeroed values until a
device attaches. A cached record is then always valid and connectable, and the whole
class of races disappears.

### Precedent already in the tree

The codebase already treats destroying the `DirconManager` as undesirable and has a
mechanism for preserving it across virtual-device recreation:

- `virtualbike::detachDirconManager()` / `attachDirconManager()`
  — `src/virtualdevices/virtualbike.cpp:564-589`
- callers that hand the surviving manager to a newly built virtual device:
  `src/devices/fakebike/fakebike.cpp:211-216`,
  `src/devices/echelonconnectsport/echelonconnectsport.cpp:654-680`

`attachDirconManager()` re-parents the manager and rewires its signals;
`detachDirconManager()` disconnects, un-parents and returns it. That is most of the
machinery the refactor needs — the change is to hoist ownership one level further, so
that no device owns it at all.

One caveat before reusing it literally: `attachDirconManager()` calls
`dirconManager->setParent(this)` (`virtualbike.cpp:570`), which is exactly what must
*not* happen once the manager outlives the device — a re-parented manager is deleted
with the virtual bike by `bluetoothdevice::setVirtualDevice()`, which deletes the old
virtual device outright (`src/devices/bluetoothdevice.cpp:257-258`). The reusable part
is the signal wiring; the parenting line has to go, or `attach` has to grow a "borrow,
do not own" mode.

### Shape of the change

1. **Move ownership out of the virtual devices.** Create the `DirconManager` once, at
   a process-lifetime scope (`homeform` is the natural holder; a singleton is the
   alternative), guarded by `dircon_yes`. The virtual bike then only attaches to it,
   using the existing `attachDirconManager()` path minus the `setParent()` line noted
   above.

2. **Make the bound device swappable.** `DirconManager::DirconManager(bluetoothdevice *t, ...)`
   captures the device at construction (`src/devices/dircon/dirconmanager.cpp:193`).
   It needs a `setDevice(bluetoothdevice *)` plus a null state that serves zeros. The
   device pointer is copied into **three** places, all of which have to be rebound —
   missing any one leaves a live object pointing at a freed device:

   - `DirconManager::bt` (`dirconmanager.h:38`), read by `currentGear()`;
   - every `CharacteristicNotifier*` built by `DM_CHAR_NOTIF_BUILD_OP`
     (`dirconmanager.cpp:191`, expanded at `:210`) — each takes `Bike` in its
     constructor;
   - the write processors `writeP2AD9`, `writePE005` and `writeP0003`
     (`dirconmanager.cpp:212-215`), which also take `Bike`.

   The write processors matter most and are the easiest to forget: they are the
   *control* path, so one left pointing at a dead device turns a client's resistance or
   power write into a use-after-free rather than a stale reading. Check `bikeProvider()`
   (`dirconmanager.cpp:258-273`) for the same non-null assumption on the notifier side.

3. **Fix the advertised profile at "bike", permanently.** The service definition is
   generated by the `DM_SERV_OP` / `DM_CHAR_OP` macro tables at
   `dirconmanager.cpp:12-86`, selected by machine type (`DM_MACHINE_TYPE_BIKE` /
   `DM_MACHINE_TYPE_TREADMILL`). With no device connected there is nothing to select
   from, so the profile has to be chosen up front. Given the scope above, choose
   `DM_MACHINE_TYPE_BIKE` unconditionally and never rebuild the service set.

   Do not implement the "rebuild if a treadmill attaches" option an earlier draft
   preferred. **The listening port is derived from the machine type**, not just the
   name: `server_base_port + DM_MACHINE_##DESC` at `dirconmanager.cpp:143`, with the
   enum ordered `WAHOO_KICKR=0, WAHOO_BLUEHR=1, WAHOO_RPM_SPEED=2, WAHOO_TREADMILL=3`
   (`dirconmanager.cpp:24-28`). Rebuilding for a treadmill therefore moves the listener
   from 36866 to 36869 and invalidates every cached record — which is precisely the
   failure this refactor exists to remove. A profile switch is not a re-announcement;
   it is a new endpoint.

   For a bike, the tables yield two processors: `WAHOO_KICKR` on 36866 (services 0x1826,
   0x1818, 0x1816) and `WAHOO_BLUEHR` on 36867 (0x180D). `WAHOO_RPM_SPEED` matches the
   bike type but has no services mapped in `DM_SERV_OP`, so no processor is built for
   it. Rouvy connects to 36866.

   Note `rouvy_compatibility` also changes the advertised name: `DM_MACHINE_OP_ROUVY`
   at `dirconmanager.cpp:30-31` replaces the whole machine table with a single
   `"ELITE AVANTI $uuid_hex$ W"` entry — still `WAHOO_KICKR`, so still port 36866.
   Whatever default is chosen has to work for both name tables.

4. **Keep the goodbye correct.** `ProviderPrivate::sayGoodbye()` is idempotent and
   fires from both `aboutToQuit` and the destructor (`provider.cpp:70-82`). With step 3
   as written the service set is never rebuilt, so nothing else is required here — the
   goodbye fires once, at quit.

   It is still worth understanding why any future rebuild must send a goodbye *before*
   probing the new records, in case the treadmill case is ever revisited: the service
   name comes from `dircon_id`, not from the instance (`dirconmanager.cpp:133-143`), so
   old and new records share an identical name. A goodbye arriving after the new
   announcement invalidates the *new* record in the client's cache. `confirm()` already
   sequences this correctly via `farewell()` when re-confirming an existing name
   (`provider.cpp:111`).

### Risks

The DIRCON service construction is the part that currently works, and this change
touches it directly. Suggested order: land the ownership move with the device still
required (no behaviour change, pure refactor), verify a ride still works, then make
the device optional in a second commit.

Three things to check on the first commit, all of which are silent if wrong:

- **Nothing else deletes the manager.** With ownership hoisted, the only remaining
  delete should be at process teardown. `bluetoothdevice::setVirtualDevice()` deletes
  the outgoing virtual device unconditionally (`bluetoothdevice.cpp:257-258`), so any
  path that still parents the manager to a virtual bike will take it down on the next
  bridge switch.
- **The port does not move.** 36866 before the change and 36866 after, with a bike
  connected and with none. If it moved, step 3 was not followed.
- **Exactly one announcement cycle at startup**, not one per device connect. The log
  line to count is `ProviderPrivate::onAnnounceTimeout repeat, remaining N` — three
  repeats, then silence until a query arrives.

Also unresolved and worth folding in only if convenient: `ftmsbike` runs its
service-discovery handler twice, subscribing to every CCCD twice and producing ~25
`errorServiceDescriptorWriteError`s per connection. Harmless today because the first
write wins, but it makes logs hard to read. Seen at
`src/devices/ftmsbike/ftmsbike.cpp:1893-1933`.

## Verifying any of this without Rouvy

A passive listener is enough to see whether QZ is announcing, and on which interface.
Bind 5353, join the group **via the specific interface**, and read:

```powershell
$local = [System.Net.IPAddress]::Parse('192.168.0.107')   # the Wi-Fi address
$udp = New-Object System.Net.Sockets.UdpClient
$udp.Client.SetSocketOption('Socket','ReuseAddress',$true)
$udp.Client.Bind((New-Object System.Net.IPEndPoint([System.Net.IPAddress]::Any,5353)))
$udp.JoinMulticastGroup([System.Net.IPAddress]::Parse('224.0.0.251'), $local)
$udp.Client.ReceiveTimeout = 500
# then $udp.Receive([ref]$remote) in a loop and dump the bytes as ASCII
```

Two traps, both of which cost time in the original session:

- **Joining the group is per interface.** Joining via the Wi-Fi address shows only
  what arrives on Wi-Fi. That is the point — it is how the wrong-interface bug was
  found — but it also means a quiet capture can mean "wrong interface", not "silent".
- **Sending has the same bug the fix addressed.** A probe socket that does not set
  `IP_MULTICAST_IF` sends its query out whatever interface the OS picks, which may
  not be the LAN. Set it explicitly:

```powershell
$udp.Client.SetSocketOption([System.Net.Sockets.SocketOptionLevel]::IP,
                            [System.Net.Sockets.SocketOptionName]::MulticastInterface,
                            $local.GetAddressBytes())
```

To see what the OS would pick on its own:

```powershell
$u = New-Object System.Net.Sockets.UdpClient
$u.Client.Bind((New-Object System.Net.IPEndPoint([System.Net.IPAddress]::Any,0)))
$u.Connect((New-Object System.Net.IPEndPoint([System.Net.IPAddress]::Parse('224.0.0.251'),5353)))
$u.Client.LocalEndPoint.Address
```

Useful log lines, in `debug-<timestamp>.log` next to the exe:

| Line | Means |
|---|---|
| `getIP fallback QHostAddress("192.168.x.y")` | A record resolved (fault 1 fixed) |
| `ProviderPrivate::publish QHostAddress(...)` | records published with that address |
| `onAnnounceTimeout repeat, remaining N` | repeated announcements firing |
| `ProviderPrivate::sayGoodbye` | goodbye sent on quit |
| `Dircon TCP Server RV true` | listener up on 36866 |
| `Prober::nameConfirmed false` | **normal** — prints the flag *before* setting it |

Rouvy's own logs are more informative than its UI:
`%USERPROFILE%\AppData\LocalLow\VirtualTraining\ROUVY\rouvy.log`, crashes in
`%LOCALAPPDATA%\Temp\VirtualTraining\ROUVY\Crashes\*\Player.log`.

## Environment notes

- Windows binaries are built on fork CI, never locally: Smart App Control is enforced
  on the dev machine and blocks `g++` and unsigned wheels. Do not suggest disabling it.
  `gh workflow run CI -R nickolas122/qz --ref <branch>`, then
  `gh run download <id> -R nickolas122/qz -n windows-binary-no-python`.
  Artifact downloads run at roughly 0.1 MB/s from this connection; a 62 MB artifact
  takes ~10 minutes.
- The CI matrix is trimmed to Android and Windows. `linux-x86-build` is disabled, and
  it is the only job that runs the test suite — including the ERG tests described in
  [AUTO-ERG-MODE.md](AUTO-ERG-MODE.md). Re-enable it before trusting any test result.
- Never push to `cagnulein/qdomyos-zwift`. `origin` is `nickolas122/qz`; the upstream
  push URL is deliberately set to `DISABLED_NO_PUSH_TO_UPSTREAM`.
- Test bike is `YPBM001264` (FTMS, plus a proprietary `fff0` service). Its control
  point never acknowledges `REQUEST_CONTROL` or `START_RESUME` - the handshake logs
  `finished without the bike ever acknowledging - control point appears write-only on
  this console` - so do not read that message as a regression.
