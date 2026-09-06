package org.cagnulen.qdomyoszwift;

import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.provider.OpenableColumns;
import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import org.cagnulen.qdomyoszwift.QLog;

public class ContentHelper {

    public static String getFileName(Context context, Uri uri) {
        if (uri == null) {
            return null;
        }

        String result = null;
        String scheme = uri.getScheme();
        if ("content".equals(scheme)) {
            QLog.d("ContentHelper", "content");
            Cursor cursor = null;
            try {
                cursor = context.getContentResolver().query(uri, new String[] {OpenableColumns.DISPLAY_NAME}, null, null, null);
                QLog.d("ContentHelper", "cursor " + cursor);
                if (cursor != null && cursor.moveToFirst()) {
                    int nameIndex = cursor.getColumnIndex(OpenableColumns.DISPLAY_NAME);
                    if (nameIndex >= 0) {
                        result = cursor.getString(nameIndex);
                        QLog.d("ContentHelper", "result " + result);
                    }
                }
            } catch (Exception e) {
                QLog.d("ContentHelper", "getFileName query failed " + e);
            } finally {
                if (cursor != null) {
                    cursor.close();
                }
            }
        }
        if ((result == null || result.isEmpty()) && uri.getLastPathSegment() != null) {
            result = uri.getLastPathSegment();
        }
        return result;
    }

    public static boolean copyContentToFile(Context context, Uri uri, String destinationPath) {
        if (context == null || uri == null || destinationPath == null || destinationPath.isEmpty()) {
            return false;
        }

        InputStream inputStream = null;
        FileOutputStream outputStream = null;
        try {
            inputStream = context.getContentResolver().openInputStream(uri);
            if (inputStream == null) {
                QLog.d("ContentHelper", "copyContentToFile null input stream for " + uri);
                return false;
            }

            outputStream = new FileOutputStream(destinationPath, false);
            byte[] buffer = new byte[8192];
            int read;
            while ((read = inputStream.read(buffer)) != -1) {
                outputStream.write(buffer, 0, read);
            }
            outputStream.flush();
            return true;
        } catch (Exception e) {
            QLog.d("ContentHelper", "copyContentToFile failed " + e);
            return false;
        } finally {
            try {
                if (inputStream != null) {
                    inputStream.close();
                }
            } catch (Exception ignored) {
            }
            try {
                if (outputStream != null) {
                    outputStream.close();
                }
            } catch (Exception ignored) {
            }
        }
    }

    public static String importContentToAppDir(Context context, Uri uri, String destinationDirPath) {
        if (context == null || uri == null || destinationDirPath == null || destinationDirPath.isEmpty()) {
            return "";
        }

        String fileName = sanitizeFileName(getFileName(context, uri));
        if (fileName.isEmpty()) {
            fileName = "imported_file";
        }

        File destinationDir = new File(destinationDirPath);
        if (!destinationDir.exists() && !destinationDir.mkdirs()) {
            QLog.d("ContentHelper", "importContentToAppDir could not create " + destinationDirPath);
            return "";
        }

        File destinationFile = new File(destinationDir, fileName);
        if (destinationFile.exists() && !destinationFile.delete()) {
            QLog.d("ContentHelper", "importContentToAppDir could not replace " + destinationFile.getAbsolutePath());
            return "";
        }

        if (!copyContentToFile(context, uri, destinationFile.getAbsolutePath())) {
            if (destinationFile.exists()) {
                destinationFile.delete();
            }
            return "";
        }

        return destinationFile.getAbsolutePath();
    }

    private static String sanitizeFileName(String value) {
        if (value == null) {
            return "";
        }

        return value.replace('\\', '_').replace('/', '_').trim();
    }
}
