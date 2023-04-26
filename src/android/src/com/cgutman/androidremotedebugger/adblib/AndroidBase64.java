package com.cgutman.androidremotedebugger.adblib;

import android.util.Base64;

import com.cgutman.adblib.AdbBase64;

public class AndroidBase64 implements AdbBase64 {
	@Override
	public String encodeToString(byte[] data) {
		return Base64.encodeToString(data, Base64.NO_WRAP);
	}
}
