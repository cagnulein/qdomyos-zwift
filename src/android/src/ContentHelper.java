package org.cagnulen.qdomyoszwift;

import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.ContentUris;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.os.Build;
import android.os.Environment;
import android.provider.MediaStore;
import android.provider.OpenableColumns;
import android.util.Log;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.OutputStream;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.Locale;
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

    public static boolean copyFileToPublicDocuments(Context context, String sourcePath, String relativePath) {
        if (context == null || sourcePath == null || sourcePath.isEmpty() || relativePath == null || relativePath.isEmpty()) {
            return false;
        }
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.Q) {
            return false;
        }
        Uri uri = null;
        try (FileInputStream input = new FileInputStream(sourcePath)) {
            String normalized = relativePath.replace('\\', '/');
            int separator = normalized.lastIndexOf('/');
            String name = separator >= 0 ? normalized.substring(separator + 1) : normalized;
            String parent = separator >= 0 ? normalized.substring(0, separator) : "";
            String relativeDirectory = Environment.DIRECTORY_DOCUMENTS + "/QZ/"
                    + (parent.isEmpty() ? "" : parent + "/");
            ContentValues values = new ContentValues();
            values.put(MediaStore.MediaColumns.DISPLAY_NAME, name);
            values.put(MediaStore.MediaColumns.MIME_TYPE, "application/octet-stream");
            values.put(MediaStore.MediaColumns.RELATIVE_PATH, relativeDirectory);
            values.put(MediaStore.MediaColumns.IS_PENDING, 1);
            ContentResolver resolver = context.getContentResolver();
            uri = resolver.insert(MediaStore.Files.getContentUri(MediaStore.VOLUME_EXTERNAL_PRIMARY), values);
            if (uri == null) {
                return false;
            }
            try (OutputStream output = resolver.openOutputStream(uri)) {
                if (output == null) {
                    return false;
                }
                byte[] buffer = new byte[64 * 1024];
                int count;
                while ((count = input.read(buffer)) != -1) {
                    output.write(buffer, 0, count);
                }
            }
            ContentValues ready = new ContentValues();
            ready.put(MediaStore.MediaColumns.IS_PENDING, 0);
            resolver.update(uri, ready, null, null);
            return true;
        } catch (Exception e) {
            if (uri != null) {
                try {
                    context.getContentResolver().delete(uri, null, null);
                } catch (Exception ignored) {
                }
            }
            Log.e("ContentHelper", "copyFileToPublicDocuments failed", e);
            return false;
        }
    }

    /**
     * Enumerate profile files stored in the public Documents/QZ directory.
     *
     * Android 10 scoped storage does not make MediaStore-owned files reliably
     * visible to Qt's FolderListModel through a file:// directory. Querying
     * MediaStore returns stable content URIs which the caller can open through
     * ContentResolver.
     */
    public static String[] listPublicDocumentsFiles(Context context, String relativeDirectory) {
        if (context == null || Build.VERSION.SDK_INT < Build.VERSION_CODES.Q) {
            return new String[0];
        }

        String directory = relativeDirectory == null ? "" : relativeDirectory.replace('\\', '/');
        while (directory.startsWith("/")) {
            directory = directory.substring(1);
        }
        while (directory.endsWith("/")) {
            directory = directory.substring(0, directory.length() - 1);
        }

        String relativePath = Environment.DIRECTORY_DOCUMENTS + "/QZ/"
                + (directory.isEmpty() ? "" : directory + "/");
        Uri collection = MediaStore.Files.getContentUri(MediaStore.VOLUME_EXTERNAL_PRIMARY);
        List<String> result = new ArrayList<>();
        String[] projection = new String[] {
                MediaStore.MediaColumns._ID,
                MediaStore.MediaColumns.DISPLAY_NAME
        };
        Cursor cursor = null;
        try {
            cursor = context.getContentResolver().query(
                    collection,
                    projection,
                    MediaStore.MediaColumns.RELATIVE_PATH + "=?",
                    new String[] {relativePath},
                    MediaStore.MediaColumns.DISPLAY_NAME + " COLLATE NOCASE DESC");
            if (cursor != null) {
                int idIndex = cursor.getColumnIndex(MediaStore.MediaColumns._ID);
                int nameIndex = cursor.getColumnIndex(MediaStore.MediaColumns.DISPLAY_NAME);
                while (cursor.moveToNext()) {
                    if (idIndex < 0 || nameIndex < 0) {
                        continue;
                    }
                    String name = cursor.getString(nameIndex);
                    if (name == null || !name.toLowerCase(Locale.US).endsWith(".qzs")) {
                        continue;
                    }
                    long id = cursor.getLong(idIndex);
                    result.add(ContentUris.withAppendedId(collection, id).toString());
                }
            }
        } catch (Exception e) {
            QLog.d("ContentHelper", "listPublicDocumentsFiles query failed " + e);
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        return result.toArray(new String[0]);
    }

    public static boolean deleteContentUri(Context context, String uriString) {
        if (context == null || uriString == null || uriString.isEmpty()) {
            return false;
        }
        try {
            Uri uri = Uri.parse(uriString);
            return uri != null && context.getContentResolver().delete(uri, null, null) > 0;
        } catch (Exception e) {
            QLog.d("ContentHelper", "deleteContentUri failed " + e);
            return false;
        }
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
