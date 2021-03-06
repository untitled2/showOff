package kickassplayer;

import javax.swing.filechooser.FileSystemView;
import java.io.*;

/**
* Customized JFileChooser to lock user inside mediafolder
* when adding mp3s to a playlist.
* <p>
* Course work. GRTOT. Spring 2010.
* <p>
* Modified 20.03.2010
*
* @author Nikita Ishkov (nikita.ishkov@uta.fi)
*/
class RestrictedFileSystemView extends FileSystemView {
    private final File[] rootDirectories;

    RestrictedFileSystemView(File rootDirectory) {
        this.rootDirectories = new File[] {rootDirectory};
    }

    RestrictedFileSystemView(File[] rootDirectories) {
        this.rootDirectories = rootDirectories;
    }

    @Override
    public File createNewFolder(File containingDir) throws IOException {
        throw new UnsupportedOperationException("Unable to create directory");
    }

    @Override
    public File[] getRoots() {
        return rootDirectories;
    }

    @Override
    public boolean isRoot(File file) {
        for (File root : rootDirectories) {
            if (root.equals(file)) {
                return true;
            }
        }
        return false;
    }

    @Override
    public File getHomeDirectory() {
        return rootDirectories[0];
    }
}