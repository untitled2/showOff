package kickassplayer;

import javax.swing.*;
import java.io.File;


/**
* Main class. Runs teh player
* <p>
* Course work. GRTOT. Spring 2010.
* <p>
* Modified 20.03.2010
*
* @author Nikita Ishkov (nikita.ishkov@uta.fi)
*/
public class Main {
        /** Mediafolder */
	private File mediaFolder;
        /** Just to check for serialized content */
	private File mediaFolderContentObject;
	private JFileChooser fc;
	private View view;
	private Model model;
        private Controller controller;
        /** Checks is mediafolder is created */
	private boolean mediaFolderContentCreated;

	public Main() {
		// Make a file chooser with default location . (current folder),
		// which can choose only directories (folders) and hides all files,
		// that are something else than folders
		fc = new JFileChooser(".");
		fc.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
		fc.setDialogTitle("Choose media folder");
		fc.setAcceptAllFileFilterUsed(false);

		// check for existing media folder content
		mediaFolderContentObject = new File("mediaFolderContent.out");
		if(!mediaFolderContentObject.exists()) {
			mediaFolderContentCreated = false;
			if (fc.showOpenDialog(null) == JFileChooser.APPROVE_OPTION)
				mediaFolder = fc.getSelectedFile();
			else
				mediaFolder = new java.io.File(".");

			// notify user about choosen media folder
			JOptionPane.showMessageDialog(null,
                                ("Media folder is now set to " + mediaFolder.toString() +
                                "\n You can not change the path").toString(),
                                "Dear user...",
                                 JOptionPane.INFORMATION_MESSAGE);
		}
		else
			mediaFolderContentCreated = true;

		// create model
		model = new Model(mediaFolder, mediaFolderContentCreated);
		view = new View(model);
	}


	public static void main(String[] args) {
		new Main();
	}
}
