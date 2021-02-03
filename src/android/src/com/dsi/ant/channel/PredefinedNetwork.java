package com.dsi.ant.channel;

/**
 * Created by harald on 24.04.17.
 */

public enum PredefinedNetwork {
    INVALID(-1),
    PUBLIC(0),
    ANT_PLUS_1(1),
    ANT_FS(2);

    private final int mRawValue;
    private static final PredefinedNetwork[] sValues = values();

    private PredefinedNetwork(int rawValue) {
        this.mRawValue = rawValue;
    }

    int getRawValue() {
        return this.mRawValue;
    }

    private boolean equals(int rawValue) {
        return rawValue == this.mRawValue;
    }

    static PredefinedNetwork create(int rawValue) {
        PredefinedNetwork code = INVALID;

        for(int i = 0; i < sValues.length; ++i) {
            if(sValues[i].equals(rawValue)) {
                code = sValues[i];
                break;
            }
        }

        return code;
    }
}
