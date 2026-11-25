package org.cagnulen.qdomyoszwift

import android.content.Context
import android.content.Intent
import android.net.Uri
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
            .apply()
        QLog.d(TAG, "Health Connect permission request finished. Granted: $grantedPermissions")
        finish()
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        // Exported aliases only open the privacy policy, even for explicit intents
        // with unexpected actions or extras. Permission requests remain internal.
        if (intent.component?.className != HealthConnectPermissionActivity::class.java.name) {
            try {
                startActivity(Intent(Intent.ACTION_VIEW, Uri.parse("https://robertoviola.cloud/privacy-policy-qdomyos-zwift/")))
            } catch (t: Throwable) {
                QLog.w(TAG, "Unable to open Health Connect privacy policy", t)
            }
            finish()
            return
        }

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
        private const val PREF_PERMISSION_PROMPT_PENDING = "permission_prompt_pending_v2"
    }
}
