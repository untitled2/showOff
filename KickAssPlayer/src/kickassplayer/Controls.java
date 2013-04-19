package kickassplayer;

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;

/**
* Layout of controls in View.
* <p>
* Course work. GRTOT. Spring 2010.
* <p>
* Modified 20.03.2010
*
* @author Nikita Ishkov (nikita.ishkov@uta.fi)
*/

public class Controls extends JPanel {
        private Controller controller;
	private JMenuBar menubar;
	private JToolBar toolbar;
        private JSlider volume;
        private JLabel volumeIconLabel;
        public static JSlider progress;
	private JCheckBoxMenuItem stopAfterCurrent, random;
        private JRadioButtonMenuItem defLoop, songLoop, plLoop;
	private JButton play, stop, pause, back, forward;
	private JMenu file, edit, help, selection, playback, order, loop;
	private JMenuItem addFile, loadPl, savePl, quit, clearPl, newPl,
                            selectAll, remove, removePl, up, down, helpI, about;

	// constructor
	public Controls(Controller c) {
		super();

                // set link to controller
                controller = c;

		// create menu
		menubar = new JMenuBar();
		menubar.add(file = new JMenu("File"));
		menubar.add(edit = new JMenu("Edit"));
		menubar.add(playback = new JMenu("Playback"));
		menubar.add(help = new JMenu("Help"));

		// add menu items to File
		file.addSeparator();
		file.add(addFile = new JMenuItem("Add file(s)", new ImageIcon(this.getClass().getResource("/toolbarButtonGraphics/general/Add16.gif"))));
		file.addSeparator();
                file.add(newPl = new JMenuItem("New playlist"));
                file.add(removePl = new JMenuItem("Remove playlist"));
		file.add(loadPl = new JMenuItem("Load playlist"));
		file.add(savePl = new JMenuItem("Save playlist"));
		file.addSeparator();
		file.add(quit = new JMenuItem("Quit", new ImageIcon(this.getClass().getResource("/toolbarButtonGraphics/general/Stop16.gif"))));
		
		// add menu items to Edit
		edit.add(clearPl = new JMenuItem("Clear playlist"));
		edit.add(selectAll = new JMenuItem("Select all"));
		// selection submenu
		selection = new JMenu("Selection");
		selection.add(remove = new JMenuItem("Remove", new ImageIcon(this.getClass().getResource("/toolbarButtonGraphics/general/Remove16.gif"))));
		// -----------------
		edit.add(selection);
		edit.add(up = new JMenuItem("Up"));
		edit.add(down = new JMenuItem("Down"));

		// add menu items to Playback
		playback.add(order = new JMenu("Order"));
                order.add(random = new JCheckBoxMenuItem("Random"));
                random.addItemListener(controller);

		playback.add(loop = new JMenu("Loop"));
                ButtonGroup group = new ButtonGroup();
                defLoop = new JRadioButtonMenuItem("Default");
                defLoop.addItemListener(controller);
                songLoop = new JRadioButtonMenuItem("Song");
                songLoop.addItemListener(controller);
                plLoop = new JRadioButtonMenuItem("Playlist");
                plLoop.addItemListener(controller);
                group.add(defLoop);
                group.add(songLoop);
                group.add(plLoop);
                loop.add(defLoop);
                loop.add(songLoop);
                loop.add(plLoop);
                defLoop.setSelected(true);

		playback.addSeparator();
		playback.add(stopAfterCurrent = new JCheckBoxMenuItem("Stop after current"));
                stopAfterCurrent.addItemListener(controller);

		// add menu items to Help
		help.add(helpI = new JMenuItem("Help", new ImageIcon(this.getClass().getResource("/toolbarButtonGraphics/general/Help16.gif"))));
		help.add(about = new JMenuItem("About", new ImageIcon(this.getClass().getResource("/toolbarButtonGraphics/general/About16.gif"))));
		
		
		// add accelerators
		stopAfterCurrent.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_K, ActionEvent.CTRL_MASK));
		selectAll.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_A, ActionEvent.CTRL_MASK));
		quit.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_Q, ActionEvent.CTRL_MASK));

                // set action commands
                newPl.setActionCommand("newPl");
                removePl.setActionCommand("removePl");
                addFile.setActionCommand("addFile");
                savePl.setActionCommand("savePl");
                loadPl.setActionCommand("loadPl");
                quit.setActionCommand("quit");
                clearPl.setActionCommand("clearPl");
                selectAll.setActionCommand("selectAll");
                remove.setActionCommand("delete");
                up.setActionCommand("up");
                down.setActionCommand("down");
                helpI.setActionCommand("help");
                about.setActionCommand("about");

                // add listener
                newPl.addActionListener(controller);
                removePl.addActionListener(controller);
                addFile.addActionListener(controller);
                savePl.addActionListener(controller);
                loadPl.addActionListener(controller);
                quit.addActionListener(controller);
                clearPl.addActionListener(controller);
                selectAll.addActionListener(controller);
                remove.addActionListener(controller);
                up.addActionListener(controller);
                down.addActionListener(controller);
                helpI.addActionListener(controller);
                about.addActionListener(controller);

		// create toolbar
		toolbar = new JToolBar();
		toolbar.setFloatable(false);
		toolbar.add(play = new JButton(new ImageIcon(this.getClass().getResource("/toolbarButtonGraphics/media/Play16.gif"))));
		toolbar.add(stop = new JButton(new ImageIcon(this.getClass().getResource("/toolbarButtonGraphics/media/Stop16.gif"))));
		toolbar.add(pause = new JButton(new ImageIcon(this.getClass().getResource("/toolbarButtonGraphics/media/Pause16.gif"))));
		toolbar.add(back = new JButton(new ImageIcon(this.getClass().getResource("/toolbarButtonGraphics/media/StepBack16.gif"))));
		toolbar.add(forward = new JButton(new ImageIcon(this.getClass().getResource("/toolbarButtonGraphics/media/StepForward16.gif"))));

                // create Jsliders for progress and volume bars
                toolbar.add(progress = new JSlider(JSlider.HORIZONTAL, 0, 100, 0));
                toolbar.add(Box.createHorizontalGlue());
                volumeIconLabel = new JLabel(new ImageIcon(this.getClass().getResource("/toolbarButtonGraphics/media/Volume16.gif")));
                toolbar.add(volumeIconLabel);
                toolbar.add(volume = new JSlider(JSlider.HORIZONTAL, -45, 0, 0));
                volume.setPreferredSize(new Dimension(40, 0));

                play.addActionListener(controller);
                play.setActionCommand("play");
                stop.addActionListener(controller);
                stop.setActionCommand("stop");
                pause.addActionListener(controller);
                pause.setActionCommand("pause");
                back.addActionListener(controller);
                back.setActionCommand("back");
                forward.addActionListener(controller);
                forward.setActionCommand("forward");
                volume.addChangeListener(controller);
                volume.setName("volume");
                progress.addChangeListener(controller);
                progress.setName("progress");

		// set menu and toolbar visible
		this.setLayout(new BorderLayout());
		this.add(menubar, BorderLayout.NORTH);
		this.add(toolbar, BorderLayout.SOUTH);
	}
}
