package kickassplayer;

import java.awt.BorderLayout;
import java.awt.Component;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.io.File;
import java.util.Calendar;
import java.util.Observable;
import java.util.Observer;
import javax.swing.*;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

/**
* Dialog to edit tags.
* Nothing interesting, routine work.
* <p>
* Course work. GRTOT. Spring 2010.
* <p>
* Modified 20.03.2010
*
* @author Nikita Ishkov (nikita.ishkov@uta.fi)
*/
public class ID3DialogEdit extends JDialog
          implements KeyListener,           // to enable apply after editing
                     ChangeListener,        // same as above, but for combobox
                     Observer {             // observer to verify user input

    private JButton apply;
    private JButton close;

    private InputValidator inputValidator;

    private JLabel artistL,
                   albumL,
                   titleL,
                   yearL,
                   genreL,
                   infoL,
                   commentL;

    private JTextField artist,
                       album,
                       title,
                       genre,
                       comment,
                       editorComp;

    private int currentYear = Calendar.getInstance().get(Calendar.YEAR);
    private JSpinner year;
    private SpinnerModel model =
                new SpinnerNumberModel(currentYear,
                                  currentYear - 1000,
                                  currentYear,
                                    1);

    private JFrame parent;

    private int index;

    private Object[] track;

    private Controller controller;

    public ID3DialogEdit(JFrame frame, Object[] trackTags, Component d, int i, Controller c) {
        super(frame, true);
        parent = frame;
        
        controller = c;

        index = i;

        inputValidator = new InputValidator();

        inputValidator.addObserver(this);

        setLocationRelativeTo(d);

        apply = new JButton("Apply");
        close = new JButton("Close");

        track = trackTags;

        JPanel info = new JPanel();
        artist = new JTextField((String) track[0]);
        album = new JTextField((String) track[1]);
        title = new JTextField((String) track[2]);
        year = new JSpinner(model);
        genre = new JTextField((String) track[4]);
        comment = new JTextField((String) track[6]);

        artist.addKeyListener(this);
        album.addKeyListener(this);
        title.addKeyListener(this);
        year.addChangeListener(this);
        genre.addKeyListener(this);
        comment.addKeyListener(this);

        int value = Integer.parseInt(track[3].toString());
        year.setValue(value != 0 ? value : 1900);
        
        year.setEditor(new JSpinner.NumberEditor(year, "#"));

        year.setEnabled(true);
        editorComp = ((JSpinner.NumberEditor)year.getEditor()).getTextField();
        editorComp.addKeyListener(this);

        artistL = new JLabel("Artist: ", JLabel.TRAILING);
        albumL = new JLabel("Album: ", JLabel.TRAILING);
        titleL = new JLabel("Title: ", JLabel.TRAILING);
        yearL = new JLabel("Year: ", JLabel.TRAILING);
        genreL = new JLabel("Genre: ", JLabel.TRAILING);
        commentL = new JLabel("Comment: ", JLabel.TRAILING);
        infoL = new JLabel("Edit ID3-information", JLabel.CENTER);

        // Shit hits the fan:
        info.setLayout(new GridBagLayout());
        GridBagConstraints leftColumn = new GridBagConstraints();
        leftColumn.fill = GridBagConstraints.HORIZONTAL;
        leftColumn.gridx = 0;
        leftColumn.weightx = 0.3;
        leftColumn.weighty = 0.3;
        leftColumn.ipadx = 30;

        GridBagConstraints rightColumn = new GridBagConstraints();
        rightColumn.fill = GridBagConstraints.HORIZONTAL;
        rightColumn.gridx = 1;
        rightColumn.ipadx = 30;
        rightColumn.weightx = 0.6;
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

        Listener kuuntelija = new Listener(this);

        apply.addActionListener(kuuntelija);
        apply.setActionCommand("apply");
        apply.setEnabled(false);
        
        close.addActionListener(kuuntelija);
        close.setActionCommand("close");

        JPanel buttons = new JPanel();
        buttons.add(apply);
        buttons.add(close);

        add(info, BorderLayout.CENTER);
        add(buttons, BorderLayout.PAGE_END);

        setResizable(false);
        pack();
        setVisible(true);
    }

    public Object[] getTags() {
        Object[] data =
        {
            artist.getText(),
            album.getText(),
            title.getText(),
            Integer.toString(((Integer)year.getValue()).intValue()),
            genre.getText(),
            comment.getText()
        };
        return data;
    }

    public Controller getController() {
        return controller;
    }

    public int getSelectedIndex() {
        return index;
    }

    public boolean editBegun() {
        return apply.isEnabled();
    }

    public JFrame ownerFrame() {
        return parent;
    }

    public void close() {
        setVisible(false);
        dispose();
    }

    public void keyTyped(KeyEvent e) { }
    public void keyPressed(KeyEvent e) { }
    public void keyReleased(KeyEvent e) {
        inputValidator.verifyInput(e, editorComp);
        if (e.getSource() != editorComp)
            apply.setEnabled(true);
    }

    public void update(Observable o, Object arg) {
        if (((String) arg).equals("invalid")) {

            editorComp.setText(String.valueOf(year.getValue()));

            JOptionPane.showMessageDialog(this,
                "Year can only be numbers " +
                "in format \"YYYY\".",
                "Invalid input",
                JOptionPane.WARNING_MESSAGE);

        }
    }

    public void stateChanged(ChangeEvent e) {
        System.out.println("stateChanged");
        apply.setEnabled(true);
    }

}


class Listener implements ActionListener {

    ID3DialogEdit owner;

    public Listener(ID3DialogEdit o) {
        owner = o;
    }

    public void actionPerformed(ActionEvent e) {
        if (e.getActionCommand().equals("apply")) {
            Object[] data = owner.getTags();

            Controller ownerC = owner.getController();

            File file = ownerC.getFileFromModel(owner.getSelectedIndex());

            Tags.writeTags(data, file.getAbsolutePath());

            // Saving song information.
            System.out.println("Song changes applied!");
            owner.close();

            ownerC.updateTagsToModel(file.getAbsolutePath());

        }
        else if (e.getActionCommand().equals("close")) {

            if (owner.editBegun()) {
                int n = JOptionPane.showConfirmDialog(
                    owner.ownerFrame(),
                    "Closing will discard any changes made.\n" +
                    "Do you want to close without saving?",
                    "Confirmation required",
                    JOptionPane.YES_NO_OPTION);

                if (n == JOptionPane.YES_NO_OPTION) {
                    owner.close();
                }

            }
            else {
                owner.setVisible(false);
                owner.dispose();
            }
            
        }
    }
}
