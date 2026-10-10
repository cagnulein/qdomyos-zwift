# Zwift and virtual gearing

## Zwift Click v2 disconnects or freezes about one minute after QZ connects on iOS. What should I try?

When using the Zwift Click v2 handshake workaround on iOS, do not leave the official Zwift app as soon as the controllers first appear connected.

1. Open the **full official Zwift app** (not Zwift Companion) and pair the trainer and Zwift Click v2 controllers.
2. Keep Zwift open for longer before closing it. Allow **at least about 3 minutes** rather than only a few seconds.
3. Then continue with the normal QZ connection procedure.

In confirmed support cases, waiting about three minutes in Zwift before continuing prevented the Click v2 controllers from freezing or disconnecting after roughly 60 seconds in QZ. In a separate confirmed case, performing the pairing/unlock step in the full Zwift app worked after using Zwift Companion had not resolved the issue.

An active Zwift subscription is not required for this handshake procedure; a Zwift account that can reach the pairing flow is sufficient.

## QZ and Zwift show different virtual gear numbers. How can I keep them aligned?

When QZ is handling a custom virtual gear table and Zwift is also showing its virtual gear UI, enable QZ's **Align Gear** option so QZ can follow the gear number reported by the virtual bike connection instead of letting the two displays drift apart.

1. Configure and enable the **Custom Gear Table** in QZ.
2. Under the **Zwift Options**, enable **Align Gear**.
3. If you want a 24-step evenly spaced virtual gear range, select the built-in **Reality Bender (24 even spaced)** gear profile.
4. Reconnect QZ and Zwift and verify that shifting changes the gear number on both sides together.

The alignment logic is active when both the custom gear table and the Zwift gear-alignment setting are enabled. QZ then reads the current gear from its virtual bike and synchronizes the local gear to it.

If QZ stops at a smaller number of gears while Zwift continues higher, check the selected custom gear profile as well as **Align Gear**. In a confirmed support case, gear alignment made QZ and Zwift shift together, and selecting the 24-step Reality Bender profile resolved the remaining mismatch in available gear range.

## Zwift auto-incline works for one rider account but not another. Can QZ support multiple Zwift accounts?

Yes. QZ's Zwift API auto-incline uses the Zwift credentials configured in QZ, so the credentials must match the Zwift rider whose session QZ should follow.

If more than one person uses the same QZ installation, create a separate QZ profile for each rider and store that rider's Zwift credentials in the corresponding profile:

1. Configure the first rider's Zwift username and password under **Zwift Options**.
2. Save that configuration as a QZ profile.
3. Create or save a second profile for the other rider and enter the second rider's Zwift credentials there.
4. Before starting the session, load the QZ profile that belongs to the rider who will use Zwift, then restart QZ if prompted.

This keeps the rest of the device configuration reusable while allowing QZ to authenticate against the correct Zwift account for API-based auto-incline.

QZ's current implementation reads the configured `zwift_username` and `zwift_password` when authenticating to Zwift, and QZ profiles save the current settings as separate configurations.

## How should I configure Chainring Size, Cog Size, and the Virtual Gear Table on a Wahoo KICKR?

When QZ virtual gearing is used with a Wahoo KICKR, **Chainring Size** and **Cog Size** describe the **physical gear that the bike is actually left in on the trainer**. They are not the virtual drivetrain you want to simulate.

For example, if the chain is physically on a 36-tooth front chainring and a 14-tooth rear sprocket while QZ handles all shifting:

- set **Chainring Size = 36**;
- set **Cog Size = 14**;
- leave the mechanical drivetrain in that physical gear during the ride.

Define the gearing you want to simulate in the **Virtual Gear Table**. Each active row contains the virtual chainring and rear-cog combination for that virtual gear. For a simple 1x drivetrain, keep the same virtual chainring in each row and enter the desired cassette sprocket sizes.

QZ uses the physical Chainring/Cog values as the reference ratio, compares each virtual gear-table ratio with that reference, and derives the virtual wheel circumference sent to the KICKR.

For a KICKR setup where the goal is to reproduce real chainring/cassette ratios, keep **Without Wheel Diameter Protocol** disabled. In the normal Wahoo path, QZ sends a wheel-circumference command whenever the virtual gear changes. Enabling **Without Wheel Diameter Protocol** switches to a different resistance/grade-based method instead.

**Virtual Wheel Size** is the base circumference used in the calculation. Start by configuring the correct physical reference gear and virtual gear table first; use Virtual Wheel Size only if you intentionally want to scale the overall gearing feel.

This physical reference does not need to change when switching between training apps such as Zwift and ROUVY, as long as the bike remains in the same mechanical gear.

## Zwift Ride or Play shifts more than one virtual gear per press. What can I try?

In QZ's Zwift Play/controller settings, enable **Buttons debouncing** and reconnect the controllers. This option filters repeated button/shift events that can otherwise cause multiple gear changes from a single press. Test with deliberate individual presses before changing other gearing settings.

The setting is implemented in the Zwift Ride/Play controller input handling and covered by controller tests. It is not a fix for an incorrect gear table or for mismatched gear numbers between Zwift and QZ; those are separate settings.
