package kickassplayer;

import java.awt.Point;
import java.awt.event.*;
import java.io.*;
import javax.swing.event.*;
import javax.swing.*;
import java.util.Vector;

/**
* Controller of MVC.
* <p>
* Course work. GRTOT. Spring 2010.
* <p>
* Modified 20.03.2010
*
* @author Nikita Ishkov (nikita.ishkov@uta.fi)
*/
public class Controller implements ActionListener, ItemListener,
               ListSelectionListener, KeyListener, ChangeListener, MouseListener {

    Model model;
    View view;

    /** Is random enabled. */
    private boolean random = false;
    /** Wheater to stop after currently played song. */
    private boolean playOnce = false;
    /* Flags for looping. */
    private boolean loopSong = false;
    /* Flags for looping. */
    private boolean loopPlaylist = false;

    private int selectedSong;
    private int playlistSize;

    private MP3Player player;

    public Controller(Model m, View v) {
        model = m;
        view = v;
    }

    public void actionPerformed(ActionEvent e) {

        System.out.println(e.getActionCommand());

        if(e.getActionCommand().equals("newPl")) {
            String s = view.showDialog("Name new playlist...", "set name");
            if(s != null && !s.equals(""))
                model.newPl(s, new Vector<File>());
        }

        else if(e.getActionCommand().equals("removePl")) {
            String s = view.getSelectedPlaylist();

            // you can not remove mediafolder
            if(!s.equals("MediaFolder"))
                model.removePl(s);
            else
                view.showNotification("You can not remove media folder!");
        }

        else if(e.getActionCommand().equals("savePl")) {
            String s = view.getSelectedPlaylist();

            // you don't save mediafolder
            if(!s.equals("MediaFolder")) {
                try {
                    model.savePl(s);
                }
                catch(Exception ex) {
                    view.showNotification(ex.toString());
                }
            }
            else
                view.showNotification("You don't save media folder!");
        }

        else if(e.getActionCommand().equals("loadPl")) {
            String s = view.showDialog("Load playlist:", "enter name");

            if(s != null && !s.equals("")) {
                try {
                    model.loadPl(s);
                }
                catch(Exception exp) {
                    view.showNotification("No such file!");
                }
            }
        }

        else if(e.getActionCommand().equals("clearPl")) {
            String s = view.getSelectedPlaylist();

            if(!s.equals("MediaFolder"))
                model.clearPl();
            else
                view.showNotification("You can't clear media folder!");
        }

        else if(e.getActionCommand().equals("selectAll")) {
            view.selectAll();
        }

        else if(e.getActionCommand().equals("up")) {
            int[] tmp = view.getSelectedSongs();
            int i = tmp[0]-1;

            model.up(i);
        }

        else if(e.getActionCommand().equals("down")) {
            int[] tmp = view.getSelectedSongs();
            int i = tmp[0]+1;

            model.down(i);
        }

        else if(e.getActionCommand().equals("quit")) {
            if (player != null)
                player.stopPlay();
            System.exit(0);
        }
        else if(e.getActionCommand().equals("help")) {
            view.showPlainNotification("Switch application n00b... \n" +
                    "Just joking, help is not implemented in this version",
                    "Help");
	}
        else if(e.getActionCommand().equals("about")) {
            view.showPlainNotification("KickAssPlayer\n" +
                                      "The ultimate tool for your audial pleasure!\n\n" +
                                      "Version 0.1 (alpha)\n\n" +
                                      "Creators:\n" +
                                      "Nikita Ishkov",
                                      "About");
	}
        else if(e.getActionCommand().equals("info")) {
            view.showTagInfo();
	}
        else if(e.getActionCommand().equals("addFile")) {
            String s = view.getSelectedPlaylist();

            if(!s.equals("MediaFolder")) {
                File[] f = view.addFiles();
                model.addSong(f);
            }
            else
                view.showNotification("You don't add files to media folder!");
        }
        
        else if(e.getActionCommand().equals("delete")) {
            delete();
        }
        else if(e.getActionCommand().equals("play")) {
            // Selected song will be played
            playlistSize = view.getCurrPlaylistSize();
            beginPlay(-1);
        }
        else if(e.getActionCommand().equals("stop")) {
            if (player != null) {
                player.stopPlay();
            }
            // Clear currently playing info.
            playingStopped();
        }
        else if(e.getActionCommand().equals("forward")) {
            if (player != null)
                playForward();
        }
        else if(e.getActionCommand().equals("back")) {
            if (player != null)
                beginPlay(selectedSong == 0 ? selectedSong : selectedSong - 1);
        }
    }

    /**  */
    public void playForward() {

        int current = view.getRowInView(selectedSong);

        if (!loopSong) {
            // Check to see if random is enabled.
            if(random) {
                int next;
                // Random song number will be counted so that it isn't the
                // same as previously played song.
                do {
                    next = (int)(playlistSize * Math.random());
                }
                while (selectedSong == next);
                beginPlay(next);
            }
            // At the end of playlist, we have know wheater to loop the playlist
            // or stop at the end.
            else {
                // Playlist looping is disabled
                if (!loopPlaylist) {
                    
                    if (current < playlistSize - 1) {
                        System.err.println("  Next song will be: " + (current + 1));
                        beginPlay(current + 1);
                    }
                    else {
                        player.stopPlay();
                        // Clear currently playing info.
                        playingStopped();
                    }
                }
                // Playlist loop enabled continues to song number 0 after reaching
                // the end of playlist.
                else
                    beginPlay(current < (playlistSize - 1) ? current + 1 : 0);
            }
        }
        // In this case we will continue playback from the same song.
        else
            beginPlay(selectedSong);
    }

    /** Start playing.
     * @param idx : If -1 selected song will be played from the playlist
     *              otherwise song was selected through next/prev buttons
     */
    public void beginPlay(int idx) {
        // First we must close all on going play-process.
        if (player != null)
            player.stopPlay();

        // Checking to see where the call came from. See below
        // '== -1' = user
        // '>=  0' = system
        if (idx == -1) {
            int i = view.getSongToPlay();
            selectedSong = i;
        }
        else
            selectedSong = view.getViewToModel(idx);

        System.out.print("Player initializing");
        System.out.print(". Selected playlist size: " + playlistSize);
        System.out.print(". Selected index: " + selectedSong);

        File song = model.getSongFile(selectedSong);

        if(song != null) {
            view.setSelectedRow(selectedSong);
            view.setPlayInfo();

            player = new MP3Player(this, model, playOnce);
            player.setFile(song);
            player.start();
        }
    }
    
    public void valueChanged(ListSelectionEvent e) {
        if (e.getValueIsAdjusting() == false) {
             String s = view.getSelectedPlaylist();
             model.setCurrPlayList(s);
        }
    }

    public void stateChanged(ChangeEvent e) {
        JSlider sj = (JSlider)e.getSource();
        if(sj.getName().equals("volume")) {
            Integer fps = (Integer)sj.getValue();
            MP3Player.setVolume(fps);
        }
    }

    public void itemStateChanged(ItemEvent e) {

        // New default instaces of possible inputs.
        JCheckBoxMenuItem item = new JCheckBoxMenuItem();
        JRadioButtonMenuItem loop = new JRadioButtonMenuItem();
        int state = 0;

        // Lets see do we have a checkbox or a radiobutton
        try {
            item = (JCheckBoxMenuItem) e.getSource();
            state = e.getStateChange();

            System.out.println(item.getText() +
                             " state changed to " +
                             (state == 1 ? "yes" : "no"));
        }
        catch (ClassCastException cce) {
            loop = (JRadioButtonMenuItem) e.getSource();
            if (e.getStateChange() == ItemEvent.SELECTED)
                System.out.println("Loop state changed to: " + loop.getText());
        }

        if (item.getText().equals("Random")) {
            random = (state == 1 ? true : false);
        }
        else if (item.getText().equals("Stop after current")) {
            playOnce = (state == 1 ? true : false);

            if (player != null)
                player.setPlayOnce(playOnce);
        }
        else if (loop.getText().equals("Default")) {
            loopSong = false;
            loopPlaylist = false;
        }
        else if (loop.getText().equals("Song")) {
            loopSong = true;
            loopPlaylist = false;
        }
        else if (loop.getText().equals("Playlist")) {
            loopSong = false;
            loopPlaylist = true;
        }
    }

    public void keyPressed(KeyEvent e) {
        // Overriden Enter.
        // Doesn't switch to the next line, but starts the song.
        if(e.getKeyCode() == KeyEvent.VK_ENTER)
            e.consume();
    }
    public void keyTyped(KeyEvent e) {
    }
    public void keyReleased(KeyEvent e) {
        System.err.println("keylistener");
        // if 'delete' pressed
        if(e.getKeyCode() == KeyEvent.VK_DELETE) {
            delete();
        }
        else if(e.getKeyCode() == KeyEvent.VK_ENTER) {
            playlistSize = view.getCurrPlaylistSize();
            beginPlay(-1);
        }
        else if(e.getKeyCode() == KeyEvent.VK_SPACE)
            if (player != null)
                player.stopPlay();
    }

    public void mouseClicked(MouseEvent e) {
        if (e.getClickCount() == 2){
            playlistSize = view.getCurrPlaylistSize();
            beginPlay(-1);
        }
    }
    public void mousePressed(MouseEvent e) {
       maybeShowPopup(e);
    }
    public void mouseReleased(MouseEvent e) {
       maybeShowPopup(e);
    }
    private void maybeShowPopup(MouseEvent e) {
        if (e.isPopupTrigger()) {
            //get the coordinates of the mouse click
            Point p = e.getPoint();

            // get the row index that contains that coordinate
            JTable table = view.getTable();
            int rowNumber = table.rowAtPoint( p );

            // get ListSelectionModel of the JTable
            ListSelectionModel modeL = table.getSelectionModel();

            // select
            modeL.setSelectionInterval( rowNumber, rowNumber );

            JPopupMenu popupMenu = view.getPopupMenu();
            popupMenu.show(e.getComponent(), e.getX(), e.getY());
        }
   }
    public void mouseEntered(MouseEvent e) {}
    public void mouseExited(MouseEvent e) {}


    public void playingStopped() {
        view.clearPlayInfo();
    }

    public void updatePlayingInfo() {
        view.setPlayInfo();
    }

    /** delete - buttons functionality. */
    public void delete() {
        String s = view.getSelectedPlaylist();

        if(!s.equals("MediaFolder"))
            model.removeSong(view.getSelectedSongs());
        else
            view.showNotification("You can not remove files from MediaFolder!");
    }

    /** Get song from MVC Model. */
    public File getFileFromModel(int idx) {
        return model.getFile(idx);
    }

    /** Write tags to MVC Model. */
    void updateTagsToModel(String absolutePath) {
        model.updateTags(absolutePath);
    }
}
