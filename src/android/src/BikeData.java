package org.cagnulen.qdomyoszwift;

import android.os.Parcel;
import android.os.Parcelable;

public class BikeData implements Parcelable {
    public static final Creator<BikeData> CREATOR = new Creator<BikeData>() {

        @Override // android.os.Parcelable.Creator
        public BikeData createFromParcel(Parcel parcel) {
            return new BikeData(parcel);
        }

        @Override // android.os.Parcelable.Creator
        public BikeData[] newArray(int i) {
            return new BikeData[i];
        }
    };
    
    // Order must match Grupetto readFromParcel order exactly
    private long mRPM;
    private long mPower;
    private long mStepperMotorPosition;
    private long mLoadCellReading;
    private int mCurrentResistance;
    private int mTargetResistance;
    
    // All other fields  
    private int mADValue;
    private int mAppliedPositionOffset;
    private String mBikeFrameSerial;
    private int mCalibrationState;
    private int mDataWriteCycle;
    private String mDataWriteDate;
    private String mDataWriteTime;
    private int mEncoderAngle;
    private int mError1Code;
    private String mError1Time;
    private int mError2Code;
    private String mError2Time;
    private int mError3Code;
    private String mError3Time;
    private int mError4Code;
    private String mError4Time;
    private int mError5Code;
    private String mError5Time;
    private int mErrorIndex;
    private int[] mErrorMap;
    private String mFWVersionNumber;
    private String mHardwareVersion;
    private int mLoadCellCalSpan;
    private float mLoadCellOffset;
    private String mLoadCellSerial;
    private String mLoadCellTable;
    private int mLoadCellTableCrc;
    private int mLoadCellTableStatus;
    private int mLoadCellTempCount;
    private String mLoadCellVersion;
    private int mLoadCellZeroData;
    private String mPSerial;
    private int mPZAFMaxResistanceSetPoint;
    private int mPZAFMinUpdateRPM;
    private int mPZAFRampDownRate;
    private int mPZAFRampUpRate;
    private byte[] mPacketData;
    private String mPacketTime;
    private int mPositionOffset;
    private int mPowerZoneAutoFollowEnabled;
    private int mPowerZoneAutoFollowPowerSetPoint;
    private int mPowerZoneAutoFollowStatus;
    private float mPowerZoneAutoFollowTargetResistance;
    private String mQSerial;
    private float mResistanceOffset;
    private int mStallThreshold;
    private int mStepperMotorEndPosition;
    private int mStepperMotorStartPosition;
    private int mSystemState;
    private float mV1Resistance;

    @Override // android.os.Parcelable
    public int describeContents() {
        return 0;
    }

    public long getRPM() {
        return this.mRPM;
    }

    public long getPower() {
        return this.mPower;
    }

    public int getTargetResistance() {
        return this.mTargetResistance;
    }

    public int getCurrentResistance() {
        return this.mCurrentResistance;
    }

    public void setRPM(long rpm) {
        this.mRPM = rpm;
    }

    public void setPower(long power) {
        this.mPower = power;
    }

    public void setTargetResistance(int resistance) {
        this.mTargetResistance = resistance;
    }

    public void setCurrentResistance(int resistance) {
        this.mCurrentResistance = resistance;
    }

    public long getStepperMotorPosition() {
        return this.mStepperMotorPosition;
    }

    public long getLoadCellReading() {
        return this.mLoadCellReading;
    }

    private BikeData(Parcel parcel) {
        readFromParcel(parcel);
    }

    @Override // android.os.Parcelable
    public void writeToParcel(Parcel parcel, int i) {
        parcel.writeLong(this.mRPM);
        parcel.writeLong(this.mPower);
        parcel.writeLong(this.mStepperMotorPosition);
        parcel.writeLong(this.mLoadCellReading);
        parcel.writeInt(this.mCurrentResistance);
        parcel.writeInt(this.mTargetResistance);
        parcel.writeString(this.mFWVersionNumber);
        parcel.writeByteArray(this.mPacketData);
        parcel.writeString(this.mPacketTime);
        parcel.writeInt(this.mStepperMotorStartPosition);
        parcel.writeInt(this.mStepperMotorEndPosition);
        parcel.writeInt(this.mCalibrationState);
        parcel.writeInt(this.mEncoderAngle);
        parcel.writeInt(this.mSystemState);
        parcel.writeInt(this.mErrorIndex);
        parcel.writeInt(this.mError1Code);
        parcel.writeString(this.mError1Time);
        parcel.writeInt(this.mError2Code);
        parcel.writeString(this.mError2Time);
        parcel.writeInt(this.mError3Code);
        parcel.writeString(this.mError3Time);
        parcel.writeInt(this.mError4Code);
        parcel.writeString(this.mError4Time);
        parcel.writeInt(this.mError5Code);
        parcel.writeString(this.mError5Time);
        parcel.writeIntArray(this.mErrorMap);
        parcel.writeString(this.mLoadCellTable);
        parcel.writeInt(this.mLoadCellTableCrc);
        parcel.writeString(this.mPSerial);
        parcel.writeString(this.mQSerial);
        parcel.writeString(this.mBikeFrameSerial);
        parcel.writeString(this.mLoadCellSerial);
        parcel.writeFloat(this.mLoadCellOffset);
        parcel.writeInt(this.mDataWriteCycle);
        parcel.writeString(this.mDataWriteDate);
        parcel.writeString(this.mDataWriteTime);
        parcel.writeInt(this.mLoadCellZeroData);
        parcel.writeInt(this.mLoadCellCalSpan);
        parcel.writeInt(this.mLoadCellTempCount);
        parcel.writeFloat(this.mResistanceOffset);
        parcel.writeInt(this.mPositionOffset);
        parcel.writeInt(this.mLoadCellTableStatus);
        parcel.writeFloat(this.mV1Resistance);
        parcel.writeString(this.mLoadCellVersion);
        parcel.writeInt(this.mAppliedPositionOffset);
        parcel.writeInt(this.mStallThreshold);
        parcel.writeString(this.mHardwareVersion);
        parcel.writeInt(this.mADValue);
        parcel.writeInt(this.mPowerZoneAutoFollowEnabled);
        parcel.writeInt(this.mPowerZoneAutoFollowPowerSetPoint);
        parcel.writeFloat(this.mPowerZoneAutoFollowTargetResistance);
        parcel.writeInt(this.mPowerZoneAutoFollowStatus);
        parcel.writeInt(this.mPZAFRampUpRate);
        parcel.writeInt(this.mPZAFRampDownRate);
        parcel.writeInt(this.mPZAFMaxResistanceSetPoint);
        parcel.writeInt(this.mPZAFMinUpdateRPM);
    }

    private void readFromParcel(Parcel parcel) {
        this.mRPM = parcel.readLong();
        this.mPower = parcel.readLong();
        this.mStepperMotorPosition = parcel.readLong();
        this.mLoadCellReading = parcel.readLong();
        this.mCurrentResistance = parcel.readInt();
        this.mTargetResistance = parcel.readInt();
        this.mFWVersionNumber = parcel.readString();
        this.mPacketData = parcel.createByteArray();
        this.mPacketTime = parcel.readString();
        this.mStepperMotorStartPosition = parcel.readInt();
        this.mStepperMotorEndPosition = parcel.readInt();
        this.mCalibrationState = parcel.readInt();
        this.mEncoderAngle = parcel.readInt();
        this.mSystemState = parcel.readInt();
        this.mErrorIndex = parcel.readInt();
        this.mError1Code = parcel.readInt();
        this.mError1Time = parcel.readString();
        this.mError2Code = parcel.readInt();
        this.mError2Time = parcel.readString();
        this.mError3Code = parcel.readInt();
        this.mError3Time = parcel.readString();
        this.mError4Code = parcel.readInt();
        this.mError4Time = parcel.readString();
        this.mError5Code = parcel.readInt();
        this.mError5Time = parcel.readString();
        int[] iArr = new int[15];
        this.mErrorMap = iArr;
        parcel.readIntArray(iArr);
        this.mLoadCellTable = parcel.readString();
        this.mLoadCellTableCrc = parcel.readInt();
        this.mPSerial = parcel.readString();
        this.mQSerial = parcel.readString();
        this.mBikeFrameSerial = parcel.readString();
        this.mLoadCellSerial = parcel.readString();
        this.mLoadCellOffset = parcel.readFloat();
        this.mDataWriteCycle = parcel.readInt();
        this.mDataWriteDate = parcel.readString();
        this.mDataWriteTime = parcel.readString();
        this.mLoadCellZeroData = parcel.readInt();
        this.mLoadCellCalSpan = parcel.readInt();
        this.mLoadCellTempCount = parcel.readInt();
        this.mResistanceOffset = parcel.readFloat();
        this.mPositionOffset = parcel.readInt();
        this.mLoadCellTableStatus = parcel.readInt();
        this.mV1Resistance = parcel.readFloat();
        this.mLoadCellVersion = parcel.readString();
        this.mAppliedPositionOffset = parcel.readInt();
        this.mStallThreshold = parcel.readInt();
        this.mHardwareVersion = parcel.readString();
        this.mADValue = parcel.readInt();
        this.mPowerZoneAutoFollowEnabled = parcel.readInt();
        this.mPowerZoneAutoFollowPowerSetPoint = parcel.readInt();
        this.mPowerZoneAutoFollowTargetResistance = parcel.readFloat();
        this.mPowerZoneAutoFollowStatus = parcel.readInt();
        this.mPZAFRampUpRate = parcel.readInt();
        this.mPZAFRampDownRate = parcel.readInt();
        this.mPZAFMaxResistanceSetPoint = parcel.readInt();
        this.mPZAFMinUpdateRPM = parcel.readInt();
    }
}
