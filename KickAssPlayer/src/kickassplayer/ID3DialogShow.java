package kickassplayer;

import java.awt.BorderLayout;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import javax.swing.*;

/**
* Dialog to show tags.
* Nothing interesting, routine work.
* <p>
* Course work. GRTOT. Spring 2010.
* <p>
* Modified 20.03.2010
*
* @author Nikita Ishkov (nikita.ishkov@uta.fi)
*/
public class ID3DialogShow extends JDialog implements ActionListener {

    private JButton edit;
    private JButton close;

    private JLabel artistL,
                   albumL,
                   titleL,
                   yearL,
                   genreL,
                   infoL,
                   commentL;

    private JLabel artist,
                   album,
                   title,
                   year,
                   genre,
                   comment;

    private View parent;

    private Object[] track;

    private int index;

    private boolean mediaFolderSelected;

    private Controller controller;

    public ID3DialogShow(View p, Object[] trackTags, int[] idx, Controller c) {
        super(p, true);
        parent = p;

        setLocationRelativeTo(parent);

        controller = c;

        index = idx[0];

        edit = new JButton("Edit");
        close = new JButton("Close");

        track = trackTags;
        
        JPanel info = new JPanel();
        artist = new JLabel((String) track[0]);
        album = new JLabel((String) track[1]);
        title = new JLabel((String) track[2]);
        year = new JLabel("" + Integer.parseInt(track[3].toString()));
        genre = new JLabel(/*(String)*/ track[4].toString());
        comment = new JLabel((String) track[6]);

        artistL = new JLabel("Artist:   ", JLabel.TRAILING);
        albumL = new JLabel("Album:   ", JLabel.TRAILING);
        titleL = new JLabel("Title:   ", JLabel.TRAILING);
        yearL = new JLabel("Year:   ", JLabel.TRAILING);
        genreL = new JLabel("Genre:   ", JLabel.TRAILING);
        commentL = new JLabel("Comment:   ", JLabel.TRAILING);
        infoL = new JLabel("Songs ID3-information", JLabel.CENTER);

        // Shit hits the fan:
        info.setLayout(new GridBagLayout());
        GridBagConstraints leftColumn = new GridBagConstraints();
        leftColumn.fill = GridBagConstraints.HORIZONTAL;
        leftColumn.gridx = 0;
        leftColumn.weightx = 0.3;
        leftColumn.weighty = 0.3;
        leftColumn.ipadx = 30;
        leftColumn.ipady = 7;

        GridBagConstraints rightColumn = new GridBagConstraints();
        rightColumn.fill = GridBagConstraints.HORIZONTAL;
        rightColumn.gridx = 1;
        rightColumn.ipadx = 30;
        rightColumn.weightx = 0.6;
        rightColumn.ipady = 7;
        leftColumn.weighty = 0.3;

        GridBagConstraints header = new GridBagConstraints();
        header.fill = GridBagConstraints.HORIZONTAL;
        header.ipady = 30;
        header.weightx = 0.1;
        header.gridwidth = 2;  // header
        header.gridx = 0;
        header.gridy = 0;
        info.add(infoL, header);

        leftColumn.gridy = 1;
        info.add(artistL, leftColumn); // Row 1, label
        rightColumn.gridy = 1;
        info.add(artist, rightColumn);  // Row 1, textfield

        leftColumn.gridy = 2;
        info.add(albumL, leftColumn);  // ...
        rightColumn.gridy = 2;
        info.add(album, rightColumn);

        leftColumn.gridy = 3;
        info.add(titleL, leftColumn);
        rightColumn.gridy = 3;
        info.add(title, rightColumn);

        leftColumn.gridy = 4;
        info.add(yearL, leftColumn);
        rightColumn.gridy = 4;
        info.add(year, rightColumn);

        leftColumn.gridy = 5;
        info.add(genreL, leftColumn);
        rightColumn.gridy = 5;
        info.add(genre, rightColumn);

        leftColumn.gridy = 6;
        info.add(commentL, leftColumn);
        rightColumn.gridy = 6;
        info.add(comment, rightColumn);

        edit.addActionListener(this);
        edit.setActionCommand("edit");
        
        close.addActionListener(this);
        close.setActionCommand("close");

        JPanel buttons = new JPanel();
        buttons.add(edit);
        buttons.add(close);

        add(info, BorderLayout.CENTER);
        add(buttons, BorderLayout.PAGE_END);

        // If selected playlist is not media library, edit will be disabled
        if (parent.getSelectedPlaylistInt() != 0)
            mediaFolderSelected = false;
        else
            mediaFolderSelected = true;

        setResizable(false);
        pack();
        setVisible(true);
    }

    public void close() {
         setVisible(false);
         dispose();
    }


    public void actionPerformed(ActionEvent e) {
        if (e.getActionCommand().equals("edit")) {
            if (mediaFolderSelected) {
                setVisible(false);
                System.out.println("Edit begining.");
                new ID3DialogEdit(parent, track, infoL, index, controller);
                close();
            }
            else {
                JOptionPane.showMessageDialog(this, "Edit can only be done in Media Folder view.");
            }
        }
        else if (e.getActionCommand().equals("close")) {
            close();
        }
    }
}
