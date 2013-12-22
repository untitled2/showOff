package kickassplayer;

import java.awt.event.KeyEvent;
import java.util.Observable;
import javax.swing.JTextField;

/**
* Input validator for tag - editing.
* <p>
* Course work. GRTOT. Spring 2010.
* <p>
* Modified 20.03.2010
*
* @author Nikita Ishkov (nikita.ishkov@uta.fi)
*/
public class InputValidator extends Observable {
    
    public InputValidator() {
        super();
    }

    public void verifyInput(KeyEvent e, JTextField spinnerTF) {

        char input = e.getKeyChar();

        if (e.getKeyCode() == KeyEvent.VK_ENTER) {
            setChanged();
            notifyObservers("valid");
        }
        else if(!(Character.isDigit(input))  // if user tries to input something
           && e.getSource() == spinnerTF) {  // else than numbers to
            setChanged();
            notifyObservers("invalid");
        }
        else if(spinnerTF.getText().length() > 4  // if user tries to input something
           && e.getSource() == spinnerTF) {  // else than numbers to
            setChanged();
            notifyObservers("invalid");
        }
        
    }

}
