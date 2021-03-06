package kickassplayer;

import java.io.File;
import org.jaudiotagger.tag.*;
import org.jaudiotagger.audio.*;
import java.util.Vector;
import org.jaudiotagger.audio.mp3.MP3File;
import org.jaudiotagger.tag.id3.ID3v23Tag;
import java.util.Arrays;

/**
* Class to read and write tags.
* <p>
* Course work. GRTOT. Spring 2010.
* <p>
* Modified 20.03.2010
*
* @author Nikita Ishkov (nikita.ishkov@uta.fi)
*/
public class Tags {
	private static MP3File mp3;
	private static Tag tag;

	public static void readTags(Vector<Object> rows, Object[] cells,
                                    Vector<File> mediaFolderContent) {

  		for(int i=0;i<mediaFolderContent.size();i++) {
			try {
				// convert from java.io.File to AudioFile
				if(mediaFolderContent.get(i) != null) {
					mp3 = (MP3File) AudioFileIO.read(mediaFolderContent.get(i));
					tag = mp3.getTag();
				}
				else
					System.err.println("suck");

                                if (tag != null) {

                                    // read tags to memory
                                    try {
                                        cells[0] = tag.getFirst(FieldKey.ARTIST);
                                    } catch (KeyNotFoundException knfe) {
                                        System.err.println("Artist field not found: "
                                                + mp3.getFile().toString());
                                        cells[0] = mp3.getFile().toString();
                                    }
                                    
                                    try {
                                        cells[1] = tag.getFirst(FieldKey.ALBUM);
                                    } catch (KeyNotFoundException knfe) {
                                        System.err.println("Album field not found: "
                                                + mp3.getFile().toString());
                                        cells[1] = "";
                                    }

                                    try {
                                        cells[2] = tag.getFirst(FieldKey.TITLE);
                                    } catch (KeyNotFoundException knfe) {
                                        System.err.println("Title field not found: "
                                                + mp3.getFile().toString());
                                        cells[2] = "";
                                    }

                                    try {
                                        cells[3] = tag.getFirst(FieldKey.YEAR);
                                        if(cells[3].equals(""))
                                            cells[3] = 0;
                                    } catch (KeyNotFoundException knfe) {
                                        System.err.println("Year field not found: "
                                                + mp3.getFile().toString());
                                        cells[3] = 0;
                                    }

                                    try {
                                        cells[4] = tag.getFirst(FieldKey.GENRE);
                                    } catch (KeyNotFoundException knfe) {
                                        System.err.println("Genre field not found: "
                                                + mp3.getFile().toString());
                                        cells[4] = "";
                                    }

                                    try {
                                        cells[5] = convertTime(mp3.getAudioHeader().getTrackLength());
                                    } catch (KeyNotFoundException knfe) {
                                        System.err.println("Track length field not found: "
                                                + mp3.getFile().toString());
                                        cells[5] = "";
                                    }

                                    try {
                                        cells[6] = tag.getFirst(FieldKey.COMMENT);
                                    } catch (KeyNotFoundException knfe) {
                                        System.err.println("Genre field not found: "
                                                + mp3.getFile().toString());
                                        cells[6] = "";
                                    }
                                }

                                // If there is no ID3 or ID2 tag availeble, song
                                // will represent artist as file name.
                                
                                if (!(mp3.hasID3v1Tag()) && !(mp3.hasID3v2Tag())) {
                                    cells[0] = mp3.getFile().getName();
                                    cells[1] = "";
                                    cells[2] = "";
                                    cells[3] = 0;
                                    cells[4] = "";
                                    cells[5] = convertTime(mp3.getAudioHeader().getTrackLength());
                                    cells[6] = "";
                               }
                               
			}
			catch(Exception e) {
				System.err.println(e.toString());
			}
                    //}
                    // add filled cells to row
                    Vector<Object> v = new Vector<Object>(Arrays.asList(cells));
                    if(rows.size()<=i)
                        rows.add(v);
                    else
                        rows.set(i, v);
		}
	}
        
        private static String convertTime(int l) {
            String m = Integer.toString(l / 60);
            String s = Integer.toString(l % 60);

            if(Integer.parseInt(s) < 10)
                s = "0" + s;

            return m + ":" + s;
        }

        public static void readSingleTag(Vector<Object> row, File file) {
            try {
               mp3 = (MP3File) AudioFileIO.read(file);
                tag = mp3.getTag();
                row.set(0, tag.getFirst(FieldKey.ARTIST));
                row.set(1, tag.getFirst(FieldKey.ALBUM));
                row.set(2, tag.getFirst(FieldKey.TITLE));
                row.set(3, tag.getFirst(FieldKey.YEAR));
                row.set(4, tag.getFirst(FieldKey.GENRE));
                row.set(5, convertTime(mp3.getAudioHeader().getTrackLength()));
                row.set(6, tag.getFirst(FieldKey.COMMENT));
            }
            catch(Exception e) {
                System.err.println(e.toString());
            }
        }

        public static void writeTags(Object[] data, String fp) {

            String filepath = fp;
            boolean new_ = false;

            try {
                if (filepath == null)
                    throw new Exception("File could not be resolved.");

                // Needs a solid way of getting the correct song
                mp3 = (MP3File) AudioFileIO.read(new File(filepath));
                tag = mp3.getTag();

                if (!(tag instanceof ID3v23Tag)) {
                    tag = new ID3v23Tag();
                    new_ = true;
                }

                tag.setField(FieldKey.ARTIST, (String) data[0]);
                tag.setField(FieldKey.ALBUM, (String) data[1]);
                tag.setField(FieldKey.TITLE, (String) data[2]);
                tag.setField(FieldKey.YEAR, (String) data[3]);
                tag.setField(FieldKey.GENRE, (String) data[4]);
                tag.setField(FieldKey.COMMENT, (String) data[5]);

                if (new_)
                    mp3.setTag(tag);

                mp3.commit();

                System.out.println("Song data update successful");

                // need to tell model of change -> all views will be updated.

            }
            catch (Exception e) {
                System.err.println("Writing tag failed: " + e.toString());
            }
        }
}
