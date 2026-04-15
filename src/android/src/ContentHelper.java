package org.cagnulen.qdomyoszwift;

import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.provider.OpenableColumns;
import org.cagnulen.qdomyoszwift.QLog;

public class ContentHelper {

    public static String getFileName(Context context, Uri uri) {
        String result = null;
        if (uri.getScheme().equals("content")) {
            QLog.d("ContentHelper", "content");
            Cursor cursor = context.getContentResolver().query(uri, null, null, null, null);
            QLog.d("ContentHelper", "cursor " + cursor);
            try {
                if (cursor != null && cursor.moveToFirst()) {
                    result = cursor.getString(cursor.getColumnIndex(OpenableColumns.DISPLAY_NAME));
                    QLog.d("ContentHelper", "result " + result);
                }
            } finally {
                cursor.close();
            }
        }
        return result;
    }
}
