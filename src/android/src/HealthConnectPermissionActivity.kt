package org.cagnulen.qdomyoszwift

import android.content.Context
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.health.connect.client.PermissionController

class HealthConnectPermissionActivity : ComponentActivity() {
    private val permissionLauncher = registerForActivityResult(
        PermissionController.createRequestPermissionResultContract()
    ) { grantedPermissions ->
        getSharedPreferences(PREFS_NAME, Context.MODE_PRIVATE)
            .edit()
            .putBoolean(PREF_PERMISSION_PROMPT_PENDING, false)
            .putBoolean(PREF_PERMISSION_PROMPT_SHOWN, true)
            .apply()
        QLog.d(TAG, "Health Connect permission request finished. Granted: $grantedPermissions")
        finish()
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        val permissions = intent.getStringArrayListExtra(EXTRA_PERMISSIONS)?.toSet().orEmpty()
        if (permissions.isEmpty()) {
            QLog.d(TAG, "Health Connect permission request skipped: no permissions")
            finish()
            return
        }

        try {
            QLog.d(TAG, "Health Connect permission launcher starting: $permissions")
            permissionLauncher.launch(permissions)
        } catch (t: Throwable) {
            getSharedPreferences(PREFS_NAME, Context.MODE_PRIVATE)
                .edit()
                .putBoolean(PREF_PERMISSION_PROMPT_PENDING, false)
                .apply()
            QLog.w(TAG, "Health Connect permission launcher failed", t)
            finish()
        }
    }

    companion object {
        const val EXTRA_PERMISSIONS = "org.cagnulen.qdomyoszwift.healthconnect.PERMISSIONS"

        private const val TAG = "HealthConnectPermissionActivity"
        private const val PREFS_NAME = "qz_health_connect"
        private const val PREF_PERMISSION_PROMPT_SHOWN = "permission_prompt_shown_v2"
        private const val PREF_PERMISSION_PROMPT_PENDING = "permission_prompt_pending_v2"
    }
}
