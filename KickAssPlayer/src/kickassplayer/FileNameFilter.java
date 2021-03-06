package kickassplayer;

import java.io.*;

/**
* Filter to select only mp3 files.
* <p>
* Course work. GRTOT. Spring 2010.
* <p>
* Modified 20.03.2010
*
* @author Nikita Ishkov (nikita.ishkov@uta.fi)
*/
public class FileNameFilter extends javax.swing.filechooser.FileFilter implements java.io.FilenameFilter {
	String ext;

	public FileNameFilter(String ext) {
		this.ext = "." + ext;
	}

	public boolean accept(File dir, String name) {
		return name.endsWith(ext);
	}
        
        public boolean accept(java.io.File pathname) {

            String S = pathname.getName().toLowerCase();
            return S.endsWith(ext);
	}

        public String getDescription() {
		return "mp3";
	}
}
