#!/usr/bin/env python3
"""Run the actual executeReadJob method against a fake Android GATT boundary.

Requires javac and java. No Android runtime or JNI is needed.
"""
import pathlib
import subprocess
import tempfile

ROOT = pathlib.Path(__file__).resolve().parents[2]
SOURCE = ROOT / 'qt-patches/android/5.15.0/qtconnectivity/src/android/bluetooth/src/org/qtproject/qt5/android/bluetooth/QtBluetoothLE.java'
text = SOURCE.read_text()
start = text.index('    private boolean executeReadJob(ReadWriteJob nextJob)')
brace = text.index('{', start)
depth = 0
for i in range(brace, len(text)):
    if text[i] == '{': depth += 1
    if text[i] == '}':
        depth -= 1
        if depth == 0:
            method = text[start:i+1]
            break
helper_start = text.index('    private boolean isC12Challenge(ReadWriteJob job)')
helper = text[helper_start:start]
timeout_start = text.index('    private class TimeoutRunnable implements Runnable')
timeout_end = text.index('    /* Pointer to the Qt object', timeout_start)
timeout = text[timeout_start:timeout_end]
harness = r'''import java.util.*;
import java.util.concurrent.atomic.AtomicInteger;
class ReadJobTest {
 static class BluetoothGattCharacteristic {
  UUID id; Service service;
  BluetoothGattCharacteristic(String s, String c) {service=new Service(s); id=UUID.fromString(c);}
  UUID getUuid(){return id;} Service getService(){return service;}
 }
 static class Service { UUID id; Service(String s){id=UUID.fromString(s);} UUID getUuid(){return id;} }
 enum GattEntryType {Characteristic, Descriptor, Service, CharacteristicValue}
 static class Entry {GattEntryType type; BluetoothGattCharacteristic characteristic; boolean valueKnown=true;}
 static class Device {String name="ERGO C12"; String getName(){return name;}}
 Device mRemoteGattDevice=new Device();
 enum IoJobType {Read, Write, Mtu}
 static class ReadWriteJob { Entry entry=new Entry(); IoJobType jobType=IoJobType.Read; }
 static class Log {static void w(String tag,String message,Exception e){} static void w(String tag,String message){}}
 static final String TAG="test"; boolean c12ChallengeTimedOut, c12ChallengeStarted;
 int HANDLE_FOR_RESET=-1, interrupted; long qtObject=1;
 void leServiceError(long object,int handle,int error){}
 AtomicInteger handleForTimeout=new AtomicInteger(-1);
 LinkedList<ReadWriteJob> readWriteQueue=new LinkedList<>(); ReadWriteJob pendingJob;
 void interruptCurrentIO(int handle){interrupted++;}
 class Gatt {int reads; boolean result=true; boolean security; Device getDevice(){return mRemoteGattDevice;}
  boolean readCharacteristic(BluetoothGattCharacteristic c){reads++;if(security)throw new SecurityException(); return result;}}
 Gatt mBluetoothGatt=new Gatt();
''' + helper + method + timeout + r'''
 static final String SERVICE="638af000-7bde-3e25-ffc5-9de9b2a0197a";
 static final String CHALLENGE="638a1104-7bde-3e25-ffc5-9de9b2a0197a";
 static void check(boolean x,String why){if(!x)throw new AssertionError(why);}
 static ReadWriteJob job(String s,String c,GattEntryType type){ReadWriteJob j=new ReadWriteJob();j.entry.type=type;j.entry.characteristic=new BluetoothGattCharacteristic(s,c);return j;}
 public static void main(String[] args){
  ReadJobTest t=new ReadJobTest(); ReadWriteJob j=job(SERVICE,CHALLENGE,GattEntryType.Characteristic);
  check(!t.executeReadJob(j),"C12 challenge must start an asynchronous GATT read");
  check(t.mBluetoothGatt.reads==1,"expected one read");
  t.c12ChallengeStarted=false; // Independent dispatch cases below.
  check(t.executeReadJob(job(SERVICE,UUID.randomUUID().toString(),GattEntryType.Characteristic)),"other characteristic must remain skipped");
  check(t.executeReadJob(job(UUID.randomUUID().toString(),CHALLENGE,GattEntryType.Characteristic)),"wrong service must be skipped");
  check(t.executeReadJob(job(SERVICE,CHALLENGE,GattEntryType.Descriptor)),"descriptors must remain skipped");
  check(t.mBluetoothGatt.reads==1,"other reads must not reach GATT");
  check(!t.executeReadJob(job("8ce5cc01-0a4d-11e9-ab14-d663bd873d93",CHALLENGE,GattEntryType.Characteristic)),"primary selected service must work"); t.c12ChallengeStarted=false;
  t.mRemoteGattDevice.name="KETTLER C12"; j.entry.valueKnown=false;
  check(t.executeReadJob(j),"discovery must not consume challenge");
  j.entry.valueKnown=true;
  t.mBluetoothGatt.result=false; check(t.executeReadJob(j),"rejected read must skip");
  t.mBluetoothGatt.security=true; check(t.executeReadJob(j),"security exception must skip");
  t.mBluetoothGatt.security=false; t.mBluetoothGatt.result=true;
  t.pendingJob=j; t.handleForTimeout.set(65537);
  TimeoutRunnable old=t.new TimeoutRunnable(65537,j);
  ReadWriteJob replacement=job(SERVICE,CHALLENGE,GattEntryType.Characteristic);
  t.pendingJob=replacement; old.run();
  check(t.pendingJob==replacement && t.handleForTimeout.get()==65537 && t.interrupted==0,
        "stale timeout must not interrupt a replacement job with the same handle");
  t.new TimeoutRunnable(65537,replacement).run();
  check(t.pendingJob==null && t.handleForTimeout.get()==-1 && t.interrupted==1,
        "live timeout must release the queue once");
  check(t.executeReadJob(j),"challenge retry must be refused after timeout");
  System.out.println("PASS C12 read dispatch, isolation, rejection, permission failure, timeout identity");
 }
}
'''
with tempfile.TemporaryDirectory() as d:
    p = pathlib.Path(d) / 'ReadJobTest.java'
    p.write_text(harness)
    subprocess.run(['javac', '-J-XX:ActiveProcessorCount=1', '-J-Xmx256m', str(p)], check=True)
    subprocess.run(['java', '-XX:ActiveProcessorCount=1', '-Xmx256m', '-cp', d, 'ReadJobTest'], check=True)
