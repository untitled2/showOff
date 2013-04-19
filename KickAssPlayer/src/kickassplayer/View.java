package kickassplayer;

import javax.swing.*;
import java.awt.*;
import java.util.Observer;
import java.util.Observable;
import javax.swing.filechooser.FileSystemView;
import java.io.File;

/**
* View of MVC.
* <p>
* Course work. GRTOT. Spring 2010.
* <p>
* Modified 20.03.2010
*
* @author Nikita Ishkov (nikita.ishkov@uta.fi)
*/
public class View extends JFrame implements Observer {
        /** List of playlists. */
        private JList pls;
        /** Model of list of playlists. */
        private DefaultListModel plsModel;
	private Model model;
	private Controls controls;
        /** Playlist. */
	private Playlist pl;
	private Bottom bottom;
        private Controller controller;

        //Kontekstivalikko ja sen itemit.
        private JPopupMenu popupmenu;
        private JMenuItem addTo, delete, info, newList;

	public View(Model m) {
		super("KickAss player");

		model = m;

		controller = new Controller(model, this);
		//controller.addObserver(this);
                model.addObserver(this);

                // create list of playlists
                // and set MediaLibrary - element as default
                pls = new JList(plsModel = new DefaultListModel());
                addToPlaylistList("MediaFolder");
                pls.setPreferredSize(new Dimension(100, 200));
                pls.setSelectedIndex(0);
                pls.setCellRenderer(new SeparatorRenderer());
                pls.setBorder(BorderFactory.createMatteBorder(1, 1, 1, 0, Color.gray));

                // add selection listener to list
                pls.addListSelectionListener(controller);

                //Kontekstivalikko.
		popupmenu = new JPopupMenu();

                //Valikkoitemit.
                info = new JMenuItem("Track Info");
                delete = new JMenuItem("Delete");
                addTo = new JMenu("Add to");
                newList = new JMenuItem("New Playlist");

                //Kuuntelijat.
                info.addActionListener(controller); info.setActionCommand("info");
                delete.addActionListener(controller); delete.setActionCommand("delete");
                newList.addActionListener(controller); newList.setActionCommand("newList");

                // add popup
                addTo.add(newList);
                popupmenu.add(addTo);
                popupmenu.add(delete);
                popupmenu.add(info);

		//make controls
		controls = new Controls(controller);
		pl = new Playlist(m, controller, popupmenu);
		bottom = new Bottom();

		// add controls to main pane
		getContentPane().setLayout(new BorderLayout());
		getContentPane().add(controls, BorderLayout.NORTH);
		getContentPane().add(pl, BorderLayout.CENTER);
		getContentPane().add(bottom, BorderLayout.SOUTH);
                getContentPane().add(pls, BorderLayout.WEST);

		// set size, that appears on the screen
		setPreferredSize(new Dimension(560, 280));
		pack();
		// you can not shrink the window smaller that this
		setMinimumSize(new Dimension(400, 150));
		// quit on close
		setDefaultCloseOperation(EXIT_ON_CLOSE);

		// draw the window in the center of the screen and set visible
		setLocationRelativeTo(null);
		setVisible(true);
	}

        /** Add playlist to the list. */
        public void addToPlaylistList(String s) {
            if(plsModel.contains(s))
                showNotification("Duplicate!\nNot saved!");
            else {
               if(pl != null)
                   pl.clearPl();
               plsModel.addElement(s);
               // set focus to created element of list
               pls.setSelectedIndex(plsModel.getSize()-1);
            }
        }

        /** Update method of Observer. */
        public void update(Observable o, Object obj) {
            if(obj.toString().equals("removePl")) {
                int i = pls.getSelectedIndex();
                pls.setSelectedIndex(i-1);
                plsModel.remove(i);
            }

            else if(obj.toString().equals("savePl"))
                showNotification("Playlist saved!");

            else if(obj.toString().equals("switchPl")) {
                pl.clearPl();
                pl.populatePl(pls.getSelectedValue().toString());
            }

            else if(obj.toString().equals("clearPl"))
                pl.clearPl();

            else if(obj.toString().equals("up"))
                pl.up();

            else if(obj.toString().equals("down"))
                pl.down();

            else if(obj.toString().equals("addFile"))
                pl.populatePl1(pls.getSelectedValue().toString());

            else if(obj.toString().equals("delete"))
                pl.remove();

            else
                // newPl or loadPl
                addToPlaylistList(obj.toString());
        }

        /** Show plain notification. */
        public void showPlainNotification(String s, String header) {
            JOptionPane.showMessageDialog(this,
            s,
            header,
            JOptionPane.PLAIN_MESSAGE);

        }

        /** Show info notification. */
        public void showNotification(String s) {
            JOptionPane.showMessageDialog(this, s);
        }

        /** Show dialog.*/
        public String showDialog(String message, String header) {
            return (String)JOptionPane.showInputDialog( this,
                                                        message,
                                                        header,
                                                        JOptionPane.PLAIN_MESSAGE );
        }

        /** Get index of selected playlist. */
        public int getSelectedPlaylistInt() {
            return pls.getSelectedIndex();
        }

        /** Get name of selected playlist. */
        public String getSelectedPlaylist() {
            return pls.getSelectedValue().toString();
        }

        /** Get selected songs of playlist. */
        public int[] getSelectedSongs() {
            return pl.getSelectedRows();
        }

        /** Get one selected song. */
        public int getSongToPlay() {
            return pl.getSelectedRow();
        }

        /** Get current playlist size. */
        public int getCurrPlaylistSize() {
            return pl.getSelectedPlaylistSize();
        }

        /** Getter for JTable. */
        public JTable getTable() {
            return pl.getTable();
        }

        /** Getter for popup menu. */
        public JPopupMenu getPopupMenu() {
            return popupmenu;
        }

        /** Select a row in playlist. */
        public void setSelectedRow(int idx) {
            pl.setSelectedRow(idx);
        }

        /** Set playling song to the bottom. */
        public void setPlayInfo() {
            String songInfo = null;

            Object[] tagArray = pl.getSelectedRowsTags();

            songInfo = tagArray[0] + " - " + tagArray[2];

            bottom.updateCurrentStatus(songInfo);
        }

        /** Unset playing song in the bottom. */
        public void clearPlayInfo() {
            bottom.clearCurrentStatus();
        }

        /** Select all items in playlist. */
        public void selectAll() {
            pl.selectAllItems();
        }

        /** Show tags. */
        public void showTagInfo() {
            new ID3DialogShow(this, pl.getSelectedRowsTags(),
                              pl.getSelectedRows(), controller);
        }

        /** Add some files from mediafolder to a playlist. */
        public File[] addFiles() {
            File[] files = null;

            // get media folder
            File mf = model.getMediaFolder();
            System.err.println(mf.toString());
            // create a file system view, that user can't change directory
            FileSystemView fsv = new RestrictedFileSystemView(mf);
            // Create a file chooser
            JFileChooser fc = new JFileChooser(mf, fsv);
            //fc.setCurrentDirectory(model.getMediaFolder());
            fc.addChoosableFileFilter(new FileNameFilter("mp3"));
            fc.setMultiSelectionEnabled(true);
            // get returned value
            int returnVal = fc.showOpenDialog(this);

            // if return value is OK
            if (returnVal == JFileChooser.APPROVE_OPTION) {
                files = fc.getSelectedFiles();
            }

            return files;
        }

        /** Remove selected files from playlist. */
        public int[] deleteFiles() {
            return pl.getSelectedRows();
        }

        public int getRowInView(int i) {
            return pl.getRowInView(i);
        }

        public int getViewToModel(int i) {
            return pl.getViewToModel(i);
        }
}
