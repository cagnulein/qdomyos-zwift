package org.cagnulen.qdomyoszwift;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.le.ScanRecord;
import android.bluetooth.le.ScanResult;
import android.os.Parcel;
import android.os.Parcelable;
import android.util.SparseArray;

import java.nio.charset.StandardCharsets;

public class ScanRecordResult implements Parcelable {
    private static final byte[] HEX_ARRAY = "0123456789ABCDEF".getBytes(StandardCharsets.US_ASCII);

    protected ScanRecordResult(Parcel in) {
        rssi = in.readInt();
        name = in.readString();
        address = in.readString();
        data = in.createByteArray();
    }

    public static final Creator<ScanRecordResult> CREATOR = new Creator<ScanRecordResult>() {
        @Override
        public ScanRecordResult createFromParcel(Parcel in) {
            return new ScanRecordResult(in);
        }

        @Override
        public ScanRecordResult[] newArray(int size) {
            return new ScanRecordResult[size];
        }
    };

    public static String bytesToHex(byte[] bytes) {
        if (bytes == null)
            return "N/A";
        byte[] hexChars = new byte[bytes.length * 2];
        for (int j = 0; j < bytes.length; j++) {
            int v = bytes[j] & 0xFF;
            hexChars[j * 2] = HEX_ARRAY[v >>> 4];
            hexChars[j * 2 + 1] = HEX_ARRAY[v & 0x0F];
        }
        return new String(hexChars, StandardCharsets.UTF_8);
    }
    private int rssi = -1;
    private byte[] data = null;
    private String name = "";
    private String address = "";
    public String toString() {
        return name + " (" + address + ")["+ rssi + "] "+bytesToHex(data);
    }

    public int getRssi() {
        return rssi;
    }

    public void setRssi(int rssi) {
        this.rssi = rssi;
    }

    public byte[] getData() {
        return data;
    }

    public void setData(byte[] data) {
        this.data = data;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getAddress() {
        return address;
    }

    public void setAddress(String address) {
        this.address = address;
    }
    public ScanRecordResult(ScanResult sres) {
        rssi = sres.getRssi();
        SparseArray<byte[]> sp = null;
        ScanRecord sr = sres.getScanRecord();
        if ((sp = sr.getManufacturerSpecificData()) != null && sp.size() > 0)
            data = sp.get(sp.keyAt(0));
        name = sr.getDeviceName();
        BluetoothDevice dev = sres.getDevice();
        if (dev!=null)
            address = dev.getAddress();
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(rssi);
        dest.writeString(name == null? "": name);
        dest.writeString(address == null? "": address);
        dest.writeByteArray(data == null? new byte[]{}: data);
    }
}
