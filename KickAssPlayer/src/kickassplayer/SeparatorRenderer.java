package kickassplayer;
import javax.swing.*;
import java.awt.*;

/**
* Separator for the list of playlists.
* <p>
* Course work. GRTOT. Spring 2010.
* <p>
* Modified 20.03.2010
*
* @author Nikita Ishkov (nikita.ishkov@uta.fi)
*/
public class SeparatorRenderer extends JLabel implements ListCellRenderer {
        public Component getListCellRendererComponent(JList list, Object value, int index,
                                              boolean isSelected, boolean cellHasFocus) {
        String s = value.toString();
        setText(s);

        if (index == 0) {
            setBorder(BorderFactory.createMatteBorder(0, 0, 1, 0, Color.DARK_GRAY));
        }
        else
            setBorder(null);

        if (isSelected) {
            setBackground(Color.LIGHT_GRAY);
            setForeground(Color.BLACK);
        }
        else {
            setBackground(list.getBackground());
            setForeground(list.getForeground());
        }


        setEnabled(list.isEnabled());
        setFont(list.getFont());
        setOpaque(true);
        return this;
    }

}
