package kickassplayer;

import javax.swing.*;
import java.util.Vector;

/**
* Overriden JTable to set cells noneditable.
* <p>
* Course work. GRTOT. Spring 2010.
* <p>
* Modified 20.03.2010
*
* @author Nikita Ishkov (nikita.ishkov@uta.fi)
*/
public class MyTable extends JTable {

	public MyTable(Vector<Object> rowData, Vector<String> columnNames) {
		super(rowData, columnNames);
	}

        @Override
	public boolean isCellEditable(int row, int column) {
		return false;
	}
}
