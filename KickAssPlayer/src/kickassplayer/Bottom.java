package kickassplayer;

import javax.swing.*;

/**
* Bottom of View.
* Shows currently playing song.
* <p>
* Course work. GRTOT. Spring 2010.
* <p>
* Modified 20.03.2010
*
* @author Nikita Ishkov (nikita.ishkov@uta.fi)
*/

public class Bottom extends JPanel {
        // JLabel to display the currently playing song.
	private JLabel currentSong;

	public Bottom() {
		super();

                setLayout(new BoxLayout(this, BoxLayout.PAGE_AXIS));

		currentSong = new JLabel("Currently playing: ");
                
                setBorder(BorderFactory.createEmptyBorder(4,4,4,4));

                add(currentSong);
	}

        /** Method for updating the satusfield when a new song is selected. */
        public void updateCurrentStatus(String song) {
            currentSong.setText("Currently playing: " + song);
        }

        /** When stop is pressed statusfield is cleared. */
        public void clearCurrentStatus() {
            currentSong.setText("Currently playing: ");
        }
}
