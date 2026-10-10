package org.cagnulen.qdomyoszwift;

import android.content.ContentResolver;
import android.content.ContentValues;
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
import java.io.InputStream;
import java.io.OutputStream;
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

    // Size in bytes reported by the provider, or -1 when it is unknown.
    public static long getFileSize(Context context, Uri uri) {
        Cursor cursor = null;
        try {
            cursor = context.getContentResolver().query(uri, new String[] {OpenableColumns.SIZE}, null, null, null);
            if (cursor != null && cursor.moveToFirst() && !cursor.isNull(0)) {
                return cursor.getLong(0);
            }
        } catch (Exception e) {
            QLog.d("ContentHelper", "getFileSize query failed " + e);
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        return -1;
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
            String normalized = relativePath.replace('\\\\', '/');
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

    public static boolean copyFileToFile(String sourcePath, String destinationPath) {
        if (sourcePath == null || sourcePath.isEmpty() || destinationPath == null || destinationPath.isEmpty()) {
            return false;
        }

        java.io.FileInputStream inputStream = null;
        java.io.FileOutputStream outputStream = null;
        try {
            inputStream = new java.io.FileInputStream(sourcePath);
            outputStream = new java.io.FileOutputStream(destinationPath, false);
            byte[] buffer = new byte[8192];
            int read;
            while ((read = inputStream.read(buffer)) != -1) {
                outputStream.write(buffer, 0, read);
            }
            outputStream.flush();
            return true;
        } catch (Exception e) {
            QLog.d("ContentHelper", "copyFileToFile failed " + e);
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

    // Copies a picked file into destinationDirPath without replacing anything there: a file with
    // the same name and size is taken as already imported, otherwise a free name gets a "_N"
    // suffix. A name taken by a file this install cannot see (left by an uninstalled copy of the
    // app) fails to open for writing, so the next suffix is tried as well.
    public static String importContentToAppDirKeepExisting(Context context, Uri uri, String fileName, long size,
                                                           String destinationDirPath) {
        if (context == null || uri == null || destinationDirPath == null || destinationDirPath.isEmpty()) {
            return "";
        }

        fileName = sanitizeFileName(fileName);
        if (fileName.isEmpty()) {
            fileName = sanitizeFileName(getFileName(context, uri));
        }
        if (fileName.isEmpty()) {
            fileName = "imported_file";
        }

        File destinationDir = new File(destinationDirPath);
        if (!destinationDir.exists() && !destinationDir.mkdirs()) {
            QLog.d("ContentHelper", "importContentToAppDirKeepExisting could not create " + destinationDirPath);
            return "";
        }

        int dot = fileName.lastIndexOf('.');
        String base = dot > 0 ? fileName.substring(0, dot) : fileName;
        String extension = dot > 0 ? fileName.substring(dot) : "";
        for (int attempt = 0; attempt < 100; attempt++) {
            String candidate = attempt == 0 ? fileName : base + "_" + attempt + extension;
            File destinationFile = new File(destinationDir, candidate);
            if (destinationFile.exists()) {
                // a same-size file counts as this workout only if the app can read it: after a
                // reinstall the copies made by the previous install are still listed but unreadable
                if (size >= 0 && destinationFile.length() == size && isReadable(destinationFile)) {
                    return destinationFile.getAbsolutePath();
                }
                continue;
            }
            if (copyContentToFile(context, uri, destinationFile.getAbsolutePath())) {
                // the same workout may already be here under another name (name_1.fit, ...):
                // keep one copy, otherwise every import multiplies them
                File twin = findIdenticalFile(destinationDir, destinationFile);
                if (twin != null) {
                    destinationFile.delete();
                    return twin.getAbsolutePath();
                }
                return destinationFile.getAbsolutePath();
            }
            if (destinationFile.exists()) {
                destinationFile.delete();
            }
        }
        QLog.d("ContentHelper", "importContentToAppDirKeepExisting gave up on " + fileName);
        return "";
    }

    // A readable file of the folder with the same bytes as file, or null.
    private static File findIdenticalFile(File dir, File file) {
        File[] others = dir.listFiles();
        if (others == null) {
            return null;
        }
        for (File other : others) {
            if (other.equals(file) || !other.isFile() || other.length() != file.length()) {
                continue;
            }
            if (sameContent(other, file)) {
                return other;
            }
        }
        return null;
    }

    private static boolean sameContent(File a, File b) {
        java.io.FileInputStream inA = null;
        java.io.FileInputStream inB = null;
        try {
            inA = new java.io.FileInputStream(a);
            inB = new java.io.FileInputStream(b);
            byte[] bufA = new byte[8192];
            byte[] bufB = new byte[8192];
            while (true) {
                int readA = readFully(inA, bufA);
                int readB = readFully(inB, bufB);
                if (readA != readB) {
                    return false;
                }
                if (readA <= 0) {
                    return true;
                }
                for (int i = 0; i < readA; i++) {
                    if (bufA[i] != bufB[i]) {
                        return false;
                    }
                }
            }
        } catch (Exception e) {
            // unreadable (a file of a previous install): not a copy this install can use
            return false;
        } finally {
            try {
                if (inA != null) {
                    inA.close();
                }
            } catch (Exception ignored) {
            }
            try {
                if (inB != null) {
                    inB.close();
                }
            } catch (Exception ignored) {
            }
        }
    }

    private static int readFully(InputStream in, byte[] buffer) throws java.io.IOException {
        int total = 0;
        while (total < buffer.length) {
            int read = in.read(buffer, total, buffer.length - total);
            if (read == -1) {
                break;
            }
            total += read;
        }
        return total;
    }

    private static boolean isReadable(File file) {
        java.io.FileInputStream stream = null;
        try {
            stream = new java.io.FileInputStream(file);
            return stream.read() != -1 || file.length() == 0;
        } catch (Exception e) {
            return false;
        } finally {
            try {
                if (stream != null) {
                    stream.close();
                }
            } catch (Exception ignored) {
            }
        }
    }

    private static String sanitizeFileName(String value) {
        if (value == null) {
            return "";
        }

        return value.replace('\\', '_').replace('/', '_').trim();
    }
}
