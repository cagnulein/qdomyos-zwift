package org.cagnulen.qdomyoszwift;

import androidx.core.content.FileProvider;

// Its own class rather than FileProvider itself: the manifest merger allows one provider
// element per class name, and another FileProvider may be declared by a library or a build
public class FitFileProvider extends FileProvider {
}
