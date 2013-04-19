package kickassplayer;

import java.io.File;
import javax.swing.*;
import java.awt.*;
import java.util.Vector;
import javax.swing.table.TableModel;
import javax.swing.table.TableRowSorter;

/**
* Implements playlist and its functionality.
* <p>
* Course work. GRTOT. Spring 2010.
* <p>
* Modified 20.03.2010
*
* @author Nikita Ishkov (nikita.ishkov@uta.fi)
*/
public class Playlist extends JPanel {
        /** Modified JTable. Indicates mediafolder playlist. */
	private static MyTable currPl;
	private JScrollPane plPane;
        /** Songs in the playlist. */
        private static Vector<Object> rows;
        private Object[] cells;
	private Vector<String> columns;
        /** mediaFolder content in java.io.File[] mode */
	private static Vector<File> mediaFolderCF;
	private Model model;
        private Controller controller;
        private RowSorter<TableModel> sorter;

	public Playlist(Model m, Controller c, JPopupMenu p) {
		super();

		//set link to Model
		model = m;
                controller = c;

		// make table
		columns = new Vector<String>(7);
		columns.add("Artist");
		columns.add("Album");
		columns.add("Title");
		columns.add("Year");
		columns.add("Genre");
		columns.add("Duration");
                columns.add("hidden");

                // copy mediafolder
		mediaFolderCF = (Vector<File>)model.getMediaFolderContent();

                // Size Changed from 6 -> 7 to accommodate COMMENT-field.
                rows = new Vector<Object>(mediaFolderCF.size());
                cells = new Object[7];

		// read tags
		Tags.readTags(rows, cells, mediaFolderCF);
                // and save them to model
                model.saveTags("MediaFolder", rows);

                // create table
		currPl = new MyTable(rows, columns);

                // add key listener
                currPl.addKeyListener(controller);

                // hide comment cell from user
                currPl.removeColumn(currPl.getColumnModel().getColumn(6));

		plPane = new JScrollPane(currPl);

                // set sorter on
                sorter = new TableRowSorter<TableModel>(currPl.getModel());
                currPl.setRowSorter(sorter);

		// add mouse click listener to JTable
		currPl.addMouseListener(controller);

                plPane.setBorder(BorderFactory.createMatteBorder(1, 1, 1, 1, Color.gray));
	

		this.setLayout(new BorderLayout());
		this.add(plPane, BorderLayout.CENTER);
                setBorder(BorderFactory.createEmptyBorder(0,4,0,4));
	}

        /** Clears current playlist. */
        public void clearPl() {
            rows.clear();
            currPl.updateUI();
        }

        /** Populates playlist after deserialization. */
        public void populatePl1(String s) {
            Tags.readTags(rows, cells, model.getPlayList(s));
            model.saveTags(s, rows);

            //populatePl(s);
            currPl.updateUI();
            sorter.allRowsChanged();
        }

        /** Populates a playlist when it is selected. */
        public void populatePl(String s) {
            Vector<File> tags = (Vector<File>)model.getPlaylistTags(s);
            if(tags != null) {
                for(int i=0;i<tags.size();i++) {
                    if(rows.size()<=i)
                        rows.add(tags.get(i));
                    else
                        rows.set(i, tags.get(i));
                }
            }
            else
                clearPl();

            currPl.updateUI();
            
            sorter.allRowsChanged();
        }

        /** Select all items in playlist. */
        public void selectAllItems() {
            currPl.selectAll();
        }

        /** Move song up one position. */
        public void up() {
            int i = currPl.getSelectedRow()-1;
            if(i+1 != 0) {
                //Object o = rows.get(currPl.getSelectedRow()-1);
                Object o = rows.get(i);
                //rows.set(i, rows.get(currPl.getSelectedRow()));
                rows.set(i, rows.get(i+1));
                rows.set(i+1, o);
                //currPl.getSelectionModel().setSelectionInterval(currPl.getSelectedRow()-1, currPl.getSelectedRow()-1);
                currPl.getSelectionModel().setSelectionInterval(i, i);
                
                currPl.updateUI();
            }
        }

        /** Move song down one position. */
        public void down() {
            int i = currPl.getSelectedRow()+1;
            if(i < rows.size()) {
                //Object o = rows.get(currPl.getSelectedRow()+1);
                Object o = rows.get(i);
                //rows.set(i, rows.get(currPl.getSelectedRow()));
                rows.set(i, rows.get(i-1));
                rows.set(i-1, o);
                //currPl.getSelectionModel().setSelectionInterval(currPl.getSelectedRow()+1, currPl.getSelectedRow()+1);
                currPl.getSelectionModel().setSelectionInterval(i, i);

                currPl.updateUI();
            }
        }

        /** Get selected songs' tags. */
        public Object[] getSelectedRowsTags() {
            int[] r = getSelectedRows();
            int row = r[0];
            row = currPl.convertRowIndexToModel(row);

            // populate an array of objects with selected songs tags
            Object[] selectedTags = new Object[7];
            for(int i=0;i<7;i++)
                selectedTags[i] = ((Vector)rows.get(row)).get(i);

            return selectedTags;
            
        }

        /** Get selected rows in playlist. */
        public int[] getSelectedRows() {
            int[] selectedRows = currPl.getSelectedRows();
            if (selectedRows != null)
                return selectedRows;
            else
                return null;
        }

        /** Get selected row in playlist. */
        public int getSelectedRow() {
            int selectedRow = currPl.getSelectedRow();
            if (selectedRow >= 0 && selectedRow < rows.size())
                return currPl.convertRowIndexToModel(selectedRow);
            else
                return 0;
        }

        /** Convert selected row to view. */
        public int getRowInView(int i) {
            if(i > 0 && i < rows.size())
                return currPl.convertRowIndexToView(i);
            else
                return 0;
        }

        /** Convert selected row to model. */
        public int getViewToModel(int i) {
            if(i<rows.size())
                return currPl.convertRowIndexToModel(i);
            else
                return rows.size()-1;
        }

        /** Getter for JTable. */
        public JTable getTable() {
            return currPl;
        }

        /** Update one rows tags. */
        public static void updateRowTags(int i) {
            Tags.readSingleTag((Vector<Object>)rows.get(i), mediaFolderCF.get(i));
            currPl.updateUI();
        }

        /** Remove song from playlist. */
        public void remove() {
            if(rows.size() > 0) {
                int[] selectedRows = currPl.getSelectedRows();

                for (int i = 0; i < selectedRows.length; i++) {
                    selectedRows[i] = currPl.convertRowIndexToModel(selectedRows[i]);
                }

                currPl.clearSelection();

                if(selectedRows != null && selectedRows.length > 0)
                   for(int i=selectedRows.length-1;i>-1;i--)
                        rows.remove(selectedRows[i]);

                currPl.updateUI();
                sorter.allRowsChanged();

                // notify model
                //model.removeSong(selectedRows);
            }
        }

        /** Get size of current playlist. */
        public int getSelectedPlaylistSize() {
            return currPl.getRowCount();
        }

        /** Select row */
         void setSelectedRow(int idx) {
            currPl.clearSelection();
            idx = currPl.convertRowIndexToView(idx);
            currPl.getSelectionModel().addSelectionInterval(idx, idx);
        }
}
