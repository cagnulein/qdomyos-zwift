#!/usr/bin/env python3
"""Exercise extracted production queue, callbacks and timeout with a fake GATT/JNI boundary.

This is a JVM test, not an Android runtime or BLE device test.
"""
import pathlib, subprocess, tempfile, sys
ROOT = pathlib.Path(__file__).resolve().parents[2]
text = (ROOT / 'qt-patches/android/5.15.0/qtconnectivity/src/android/bluetooth/src/org/qtproject/qt5/android/bluetooth/QtBluetoothLE.java').read_text()
def extract(signature):
    start = text.index(signature)
    brace = text.index('{', start)
    depth = 0
    for i in range(brace, len(text)):
        if text[i] == '{': depth += 1
        elif text[i] == '}':
            depth -= 1
            if depth == 0: return text[start:i+1].replace('android.bluetooth.', '').replace('QtBluetoothLE.this', 'QueueTest.this')
    raise ValueError(signature)
parts = [extract(s) for s in [
    'private class TimeoutRunnable implements Runnable',
    'private void interruptCurrentIO(', 'private void performNextIOThreaded(',
    'private void performNextIO()', 'private void performNextIO(BluetoothGatt expectedGatt)', 'private boolean executeWriteJob(',
    'private boolean isC12Challenge(', 'private boolean executeReadJob(',
    'private int modifiedReadWriteHandle(', 'private void resetData()',
    'public void onCharacteristicRead(', 'public void onCharacteristicWrite(', 'public void onDescriptorWrite(',
    ]]
# Test-only pauses are injected into the extracted callback, not the production file.
parts[10] = parts[10].replace('            synchronized (readWriteQueue) {\n                if (gatt != mBluetoothGatt || pendingJob != completedJob)', '            pauseRead();\n            synchronized (readWriteQueue) {\n                if (gatt != mBluetoothGatt || pendingJob != completedJob)', 1)
parts[12] = parts[12].replace('            int errorCode;', '            pauseDescriptor();\n            int errorCode;', 1)
java = r'''import java.util.*;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;
class QueueTest {
 static final String S="638af000-7bde-3e25-ffc5-9de9b2a0197a", C="638a1104-7bde-3e25-ffc5-9de9b2a0197a";
 static final String TAG="test"; static final int HANDLE_FOR_RESET=-1,HANDLE_FOR_MTU_EXCHANGE=-2,RUNNABLE_TIMEOUT=100;
 static class Log {static void w(String a,String b){} static void w(String a,String b,Exception e){} static void d(String a,String b){}}
 static class BluetoothGattService {UUID uuid=UUID.fromString(S); UUID getUuid(){return uuid;}}
 static class BluetoothGattCharacteristic {static final int WRITE_TYPE_NO_RESPONSE=1; UUID uuid=UUID.fromString(C); BluetoothGattService svc=new BluetoothGattService(); byte[] value={1,2}; int writeType;
  UUID getUuid(){return uuid;} BluetoothGattService getService(){return svc;} int getProperties(){return 2;} byte[] getValue(){return value;} int getWriteType(){return writeType;} void setWriteType(int x){writeType=x;} boolean setValue(byte[] v){value=v;return true;}}
 static class BluetoothGattDescriptor {UUID uuid=UUID.fromString("00002902-0000-1000-8000-00805f9b34fb"); BluetoothGattCharacteristic c; byte[] value; BluetoothGattDescriptor(BluetoothGattCharacteristic c){this.c=c;} UUID getUuid(){return uuid;} BluetoothGattCharacteristic getCharacteristic(){return c;} boolean setValue(byte[] v){value=v;return true;} byte[] getValue(){return value;}}
 static class Device {String getName(){return "ERGO C12";}}
 static class BluetoothProfile {static final int STATE_DISCONNECTED=0,STATE_CONNECTED=2;}
 class BluetoothGatt {static final int GATT_SUCCESS=0,GATT_FAILURE=257; boolean closed; void close(){closed=true;} List<BluetoothGattService> getServices(){return Arrays.asList(new BluetoothGattService());} Device getDevice(){return new Device();} boolean readOk=true, writeOk=true; List<String> calls=new ArrayList<>();
  boolean readCharacteristic(BluetoothGattCharacteristic c){calls.add("read");return readOk;}
  boolean writeDescriptor(BluetoothGattDescriptor d){calls.add("descriptor");return writeOk;}
  boolean writeCharacteristic(BluetoothGattCharacteristic c){calls.add("write");return writeOk;}
  boolean setCharacteristicNotification(BluetoothGattCharacteristic c,boolean b){return true;}}
 enum GattEntryType {Service,Characteristic,CharacteristicValue,Descriptor} enum IoJobType {Read,Write,Mtu}
 static class GattEntry {GattEntryType type; boolean valueKnown=true; BluetoothGattCharacteristic characteristic; BluetoothGattDescriptor descriptor; int endHandle,associatedServiceHandle;}
 static class ReadWriteJob {GattEntry entry; IoJobType jobType; int requestedWriteType; byte[] newValue={1,0};}
 class Looper {void quitSafely(){}}
 class Handler {Looper getLooper(){return new Looper();} Runnable timer; void removeCallbacksAndMessages(Object x){timer=null;} void postDelayed(Runnable r,int t){timer=r;} void post(Runnable r){r.run();}}
 final Handler timeoutHandler=new Handler(); Handler mHandler=null; Object mCharacteristicConstructor=null; BluetoothGatt mBluetoothGatt=new BluetoothGatt();
 final LinkedList<ReadWriteJob> readWriteQueue=new LinkedList<>(); ReadWriteJob pendingJob; boolean c12ChallengeTimedOut, c12ChallengeStarted; AtomicInteger handleForTimeout=new AtomicInteger(-1);
 List<GattEntry> entries=new ArrayList<>(); Map<Object,Object> uuidToEntry=new HashMap<>(); List<Object> servicesToBeDiscovered=new ArrayList<>();
 UUID clientCharacteristicUuid=UUID.fromString("00002902-0000-1000-8000-00805f9b34fb"); long qtObject=1; List<String> events=new ArrayList<>();
 BluetoothGattCharacteristic cloneChararacteristic(BluetoothGattCharacteristic c){return c;} boolean executeMtuExchange(){return true;}
 boolean resetDuringLookup=false; int handleForCharacteristic(BluetoothGattCharacteristic c){for(int i=0;i<entries.size();i++)if(entries.get(i).characteristic==c){if(resetDuringLookup){resetDuringLookup=false;resetData();}return i;}return -1;}
 int handleForDescriptor(BluetoothGattDescriptor d){for(int i=0;i<entries.size();i++)if(entries.get(i).descriptor==d)return i;return -1;}
 void finishCurrentServiceDiscovery(int h){} void finishCurrentServiceDiscovery(int h,GattEntry e){} void leCharacteristicRead(long o,String s,int h,String c,int p,byte[] v){events.add("read:"+h);}
 void leCharacteristicWritten(long o,int h,byte[] v,int err){events.add("write:"+err);} void leDescriptorRead(long o,String s,String c,int h,String d,byte[] v){} void leDescriptorWritten(long o,int h,byte[] v,int err){events.add("descriptor:"+err);if(pauseDescriptorJniEnabled)pause();}
 void leServiceError(long o,int h,int e){events.add("error:"+h+":"+e);}
 void leConnectionStateChange(long o,int e,int s){events.add("state:"+s);} void leServicesDiscovered(long o,int e,String s){events.add("services");} void scheduleMtuExchange(){events.add("mtu");}
''' + '\n'.join(parts) + r'''
 CountDownLatch paused=new CountDownLatch(1), resume=new CountDownLatch(1); boolean pauseReadEnabled, pauseDescriptorEnabled, pauseDescriptorJniEnabled; Throwable callbackFailure;
 void pauseRead(){if(pauseReadEnabled) pause();} void pauseDescriptor(){if(pauseDescriptorEnabled) pause();}
 void pause(){paused.countDown();try{if(!resume.await(5,TimeUnit.SECONDS))throw new AssertionError("resume timeout");}catch(InterruptedException e){throw new RuntimeException(e);}}
 Thread callback(Runnable r){Thread th=new Thread(()->{try{r.run();}catch(Throwable x){callbackFailure=x;}});th.start();return th;}
 void waitPaused() throws Exception {check(paused.await(5,TimeUnit.SECONDS),"callback did not reach pause");}
 void finish(Thread th) throws Exception {resume.countDown();th.join(5000);check(!th.isAlive(),"callback deadlocked");if(callbackFailure!=null)throw new AssertionError("callback failed",callbackFailure);}
 static void readResetRace() throws Exception {QueueTest t=new QueueTest();BluetoothGattCharacteristic c=new BluetoothGattCharacteristic();t.entries.add(characteristic(c));t.add(t.entries.get(0),IoJobType.Read);t.performNextIO();BluetoothGatt old=t.mBluetoothGatt;t.pauseReadEnabled=true;Thread th=t.callback(()->t.onCharacteristicRead(old,c,0));t.waitPaused();t.resetData();t.mBluetoothGatt=t.new BluetoothGatt();t.finish(th);}
 static void descriptorResetRace() throws Exception {QueueTest t=new QueueTest();BluetoothGattCharacteristic c=new BluetoothGattCharacteristic();BluetoothGattDescriptor d=new BluetoothGattDescriptor(c);t.entries.add(descriptor(d));t.add(t.entries.get(0),IoJobType.Write);t.performNextIO();BluetoothGatt old=t.mBluetoothGatt;t.pauseDescriptorEnabled=true;Thread th=t.callback(()->t.onDescriptorWrite(old,d,0));t.waitPaused();t.resetData();t.mBluetoothGatt=t.new BluetoothGatt();BluetoothGattCharacteristic c2=new BluetoothGattCharacteristic();t.entries.add(characteristic(c2));ReadWriteJob newer=t.add(t.entries.get(0),IoJobType.Read);t.performNextIO();BluetoothGattDescriptor d2=new BluetoothGattDescriptor(c2);t.entries.add(descriptor(d2));t.add(t.entries.get(1),IoJobType.Write);t.finish(th);check(t.pendingJob==newer,"old descriptor callback cleared new pending job");check(t.mBluetoothGatt.calls.equals(Arrays.asList("read")),"old callback advanced new queue");t.onCharacteristicRead(t.mBluetoothGatt,c2,0);check(t.mBluetoothGatt.calls.equals(Arrays.asList("read","descriptor")),"new callback must advance its own queue");}
 static void descriptorJniRace() throws Exception {QueueTest t=new QueueTest();BluetoothGattCharacteristic c=new BluetoothGattCharacteristic();BluetoothGattDescriptor d=new BluetoothGattDescriptor(c);t.entries.add(descriptor(d));t.add(t.entries.get(0),IoJobType.Write);t.performNextIO();BluetoothGatt old=t.mBluetoothGatt;t.pauseDescriptorJniEnabled=true;Thread th=t.callback(()->t.onDescriptorWrite(old,d,0));t.waitPaused();t.resetData();t.mBluetoothGatt=t.new BluetoothGatt();BluetoothGattCharacteristic c2=new BluetoothGattCharacteristic();t.entries.add(characteristic(c2));t.add(t.entries.get(0),IoJobType.Read);t.finish(th);check(t.mBluetoothGatt.calls.isEmpty(),"old JNI continuation advanced new connection");t.performNextIO();check(t.mBluetoothGatt.calls.equals(Arrays.asList("read")),"new connection can advance itself");}
 static void check(boolean v,String s){if(!v)throw new AssertionError(s);}
 ReadWriteJob add(GattEntry e,IoJobType type){ReadWriteJob j=new ReadWriteJob();j.entry=e;j.jobType=type;readWriteQueue.add(j);return j;}
 static GattEntry characteristic(BluetoothGattCharacteristic c){GattEntry e=new GattEntry();e.type=GattEntryType.Characteristic;e.characteristic=c;return e;}
 static GattEntry descriptor(BluetoothGattDescriptor d){GattEntry e=new GattEntry();e.type=GattEntryType.Descriptor;e.descriptor=d;return e;}
 static void retryAfterGattError(){QueueTest t=new QueueTest();BluetoothGattCharacteristic c=new BluetoothGattCharacteristic();t.entries.add(characteristic(c));t.add(t.entries.get(0),IoJobType.Read);t.performNextIO();t.onCharacteristicRead(t.mBluetoothGatt,c,1);t.add(t.entries.get(0),IoJobType.Read);t.performNextIO();check(t.mBluetoothGatt.calls.equals(Arrays.asList("read")),"accepted read error requires reconnect: "+t.mBluetoothGatt.calls);}
 static void resetDuringWrite(){QueueTest t=new QueueTest();BluetoothGattCharacteristic c=new BluetoothGattCharacteristic();t.entries.add(characteristic(c));t.add(t.entries.get(0),IoJobType.Write);t.performNextIO();t.resetDuringLookup=true;t.onCharacteristicWrite(t.mBluetoothGatt,c,0);check(t.pendingJob==null && t.events.isEmpty(),"reset during write callback must not dereference null or emit completion");}
 static void retryImmediate(){QueueTest t=new QueueTest();BluetoothGattCharacteristic c=new BluetoothGattCharacteristic();t.entries.add(characteristic(c));t.mBluetoothGatt.readOk=false;t.add(t.entries.get(0),IoJobType.Read);t.performNextIO();t.mBluetoothGatt.readOk=true;t.add(t.entries.get(0),IoJobType.Read);t.performNextIO();check(t.mBluetoothGatt.calls.equals(Arrays.asList("read","read")) && t.pendingJob!=null,"unaccepted read can retry");}
 static void timeout() {QueueTest t=new QueueTest(); BluetoothGattCharacteristic c=new BluetoothGattCharacteristic(); t.entries.add(characteristic(c)); t.add(t.entries.get(0),IoJobType.Read);t.performNextIO();t.timeoutHandler.timer.run();
  check(t.events.equals(Arrays.asList("error:1:5")),"timeout must report characteristic read error exactly once: "+t.events); }
 static void ordering(){QueueTest t=new QueueTest();BluetoothGattCharacteristic c=new BluetoothGattCharacteristic();BluetoothGattDescriptor d=new BluetoothGattDescriptor(c);t.entries.add(descriptor(d));t.entries.add(characteristic(c));t.add(t.entries.get(0),IoJobType.Write);t.add(t.entries.get(1),IoJobType.Read);t.performNextIO();
  check(t.mBluetoothGatt.calls.equals(Arrays.asList("descriptor")),"read must wait for descriptor");t.onDescriptorWrite(t.new BluetoothGatt(),d,0);check(t.mBluetoothGatt.calls.size()==1,"old GATT must not release queue");t.onDescriptorWrite(t.mBluetoothGatt,d,0);check(t.mBluetoothGatt.calls.equals(Arrays.asList("descriptor","read")),"read must follow descriptor callback");t.onCharacteristicRead(t.mBluetoothGatt,c,0);check(t.events.equals(Arrays.asList("descriptor:0","read:2")),"successful callbacks: "+t.events);}
 static void reject(){QueueTest t=new QueueTest();BluetoothGattCharacteristic c=new BluetoothGattCharacteristic();t.entries.add(characteristic(c));t.mBluetoothGatt.readOk=false;t.add(t.entries.get(0),IoJobType.Read);t.add(t.entries.get(0),IoJobType.Read);t.performNextIO();check(t.events.equals(Arrays.asList("error:1:5","error:1:5")),"immediate failures must report and drain: "+t.events);}
 static void duplicate(){QueueTest t=new QueueTest();BluetoothGattCharacteristic c=new BluetoothGattCharacteristic();t.entries.add(characteristic(c));t.add(t.entries.get(0),IoJobType.Read);t.add(t.entries.get(0),IoJobType.Read);t.performNextIO();t.onCharacteristicRead(t.mBluetoothGatt,c,0);t.onCharacteristicRead(t.mBluetoothGatt,c,0);check(Collections.frequency(t.events,"read:1")==1,"duplicate callback cannot complete another challenge: "+t.events);}
 static void reconnect(){QueueTest t=new QueueTest();BluetoothGattCharacteristic c=new BluetoothGattCharacteristic();t.entries.add(characteristic(c));t.add(t.entries.get(0),IoJobType.Read);t.performNextIO();Runnable oldTimer=t.timeoutHandler.timer;BluetoothGatt oldGatt=t.mBluetoothGatt;t.resetData();t.mBluetoothGatt=t.new BluetoothGatt();t.entries.add(characteristic(c));t.add(t.entries.get(0),IoJobType.Read);t.performNextIO();t.onCharacteristicRead(oldGatt,c,0);oldTimer.run();check(t.events.isEmpty() && t.pendingJob!=null,"old callbacks and timeouts must not touch new connection");t.onCharacteristicRead(t.mBluetoothGatt,c,0);check(t.events.equals(Arrays.asList("read:1")),"new connection read must complete");}
 static void failures(){QueueTest t=new QueueTest();BluetoothGattCharacteristic c=new BluetoothGattCharacteristic();BluetoothGattDescriptor d=new BluetoothGattDescriptor(c);t.entries.add(descriptor(d));t.entries.add(characteristic(c));t.add(t.entries.get(0),IoJobType.Write);t.add(t.entries.get(1),IoJobType.Read);t.performNextIO();t.onDescriptorWrite(t.mBluetoothGatt,d,1);check(t.mBluetoothGatt.calls.equals(Arrays.asList("descriptor","read")),"failed descriptor callback must drain queue");t.onCharacteristicRead(t.mBluetoothGatt,c,1);check(t.events.equals(Arrays.asList("descriptor:3","error:2:5")),"callback failures: "+t.events);}
 static void late(){QueueTest t=new QueueTest();BluetoothGattCharacteristic c=new BluetoothGattCharacteristic();BluetoothGattDescriptor d=new BluetoothGattDescriptor(c);t.entries.add(characteristic(c));t.entries.add(descriptor(d));t.add(t.entries.get(0),IoJobType.Read);t.add(t.entries.get(1),IoJobType.Write);t.performNextIO();t.timeoutHandler.timer.run();check(t.events.equals(Arrays.asList("error:1:5")) && t.mBluetoothGatt.calls.equals(Arrays.asList("read","descriptor")),"timeout must report and advance queue");t.onCharacteristicRead(t.mBluetoothGatt,c,0);check(t.pendingJob!=null && t.events.size()==1,"late callback must not release descriptor");t.onDescriptorWrite(t.mBluetoothGatt,d,0);check(t.events.equals(Arrays.asList("error:1:5","descriptor:0")),"descriptor completion after late read");}
 public static void main(String[] args) throws Exception {switch(args[0]){case "descriptorJniRace":descriptorJniRace();break;case "readResetRace":readResetRace();break;case "descriptorResetRace":descriptorResetRace();break;case "retryImmediate":retryImmediate();break;case "resetDuringWrite":resetDuringWrite();break;case "retryAfterGattError":retryAfterGattError();break;case "timeout":timeout();break;case "ordering":ordering();break;case "reject":reject();break;case "duplicate":duplicate();break;case "reconnect":reconnect();break;case "failures":failures();break;case "late":late();break;}System.out.println("PASS "+args[0]);}
}
'''
with tempfile.TemporaryDirectory() as d:
    p=pathlib.Path(d)/'QueueTest.java';p.write_text(java)
    subprocess.run(['javac','-J-XX:ActiveProcessorCount=1','-J-Xmx256m',str(p)],check=True)
    for case in (sys.argv[1:] or ['descriptorJniRace','readResetRace','descriptorResetRace','retryImmediate','resetDuringWrite','retryAfterGattError','ordering','reject','timeout','duplicate','reconnect','failures','late']):
        result=subprocess.run(['java','-XX:ActiveProcessorCount=1','-Xmx256m','-cp',d,'QueueTest',case])
        if result.returncode: raise SystemExit(result.returncode)
