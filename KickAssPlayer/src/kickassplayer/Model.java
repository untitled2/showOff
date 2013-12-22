package kickassplayer;

import java.util.Observable;
import java.io.*;
import java.util.Vector;
import java.util.Dictionary;
import java.util.Hashtable;
import java.util.Enumeration;

/**
* Model of MVC.
* <p>
* Course work. GRTOT. Spring 2010.
* <p>
* Modified 20.03.2010
*
* @author Nikita Ishkov (nikita.ishkov@uta.fi)
*/
public class Model extends Observable implements Serializable {
    
	private String[] mediaFolderContent;
	private File mediaFolder;
	private Vector<File> mediaFolderContentF;
        private Dictionary playlistList;
        private Dictionary playlistTags;
        private String currentPlaylist;

	public Model(File mediaFolder, boolean mediaFolderContentCreated) {
            playlistList = new Hashtable();
            playlistTags = new Hashtable();

            this.mediaFolder = mediaFolder;

            // if there is no serialized mediaFolderContent yet, create it
            if(!mediaFolderContentCreated) {
                try {
                    initMediaFolder();
                } catch(Exception e) {}
            }
            // if file exists, deserialize it
            else {
                try {
                    mediaFolderContentF = (Vector<File>)deserialize("mediaFolderContent.out");
                    this.mediaFolder = (File)deserialize("mediaFolderPath");
                } catch(Exception e) {}
            }

            // add mediafolder to list of playlists
            newPl("MediaFolder", mediaFolderContentF);
            currentPlaylist = "MediaFolder";
	}

        /** Init mediafolder. */
	private void initMediaFolder() throws IOException {
		// check if we got a dir, then push all *.mp3 files into the String-table
		FilenameFilter mp3 = new FileNameFilter("mp3");
		if(mediaFolder.isDirectory())
			mediaFolderContent = mediaFolder.list(mp3);

		// convert String[] to java.io.File[]
		convert();

		// serialize mediaFolderContent (type File)
                serialize(mediaFolderContentF, "mediaFolderContent.out");
                serialize(mediaFolder, "mediaFolderPath");
	}

        /** Make new playlist. */
        public void newPl(String s, Vector<File> vf) {
            playlistList.put(s, vf);

            setChanged();
            notifyObservers(s);
        }

        /** Update tags in all playlists. */
        public void updateTags(String s) {

            Enumeration e = playlistList.keys();
            Enumeration eT = playlistTags.keys();

            while(eT.hasMoreElements()) {
                Vector<Object> vo = (Vector<Object>)playlistList.get(e.nextElement());
                Vector<Object> voTag = (Vector<Object>)playlistTags.get(eT.nextElement());

                for(int i=0;i<vo.size();i++) {
                    System.err.println(i);
                    if(vo.get(i).toString().equals(s)) {
                        vo.set(i, new File(s));

                        Tags.readSingleTag((Vector<Object>)voTag.get(i), new File(s));
                        System.err.println("Now editing: " + voTag.toString());
                        break;
                    }
                }
            }
        }

        /** Get position of a song. */
        public File getSongFile(int idx) {
            if(idx >= 0 && idx < getPlayList(currentPlaylist).size())
                return (getPlayList(currentPlaylist)).get(idx);
            else
                return null;
        }

        /** Get tags of a song. */
        public Object getSongTags(File f) {
            Vector<File> mf = (Vector<File>)playlistList.get("MediaFolder");
            Vector<Object> mfTags = (Vector<Object>)playlistTags.get("MediaFolder");
            
            return mfTags.get(mf.indexOf(f));
        }

        /** Get some playlist. */
        public Vector<File> getPlayList(String s) {
            return (Vector<File>)playlistList.get(s);
        }

        /** Serialize a playlist. */
	public void serialize( Object obj, String s) throws IOException {
            FileOutputStream fos = new FileOutputStream(s);
            ObjectOutputStream oos = new ObjectOutputStream(fos);
            
            oos.writeObject(obj);
            oos.flush();
            oos.close();
        }

        /** Deserialize a playlist. */
        public Object deserialize(String s) throws Exception {
		FileInputStream fis = new FileInputStream(s);
		ObjectInputStream oin = new ObjectInputStream(fis);
		Object obj = oin.readObject();

                return obj;
	}

        /** Convert String[] to Vector<File>. */
	private void convert() {
		// convert String[] to java.io.File
		mediaFolderContentF = new Vector<File>();

		for(int i=0;i<mediaFolderContent.length;i++) {
			mediaFolderContentF.add(i, new File((mediaFolder + "/" + mediaFolderContent[i]).toString()));
		}
	}

        /** Get song from mediafolder. */
        public File getFile(int idx) {
            return mediaFolderContentF.get(idx);
        }

        /** Get mediafolder. */
        public Vector<File> getMediaFolderContent() {
            return mediaFolderContentF;
        }

        /** Save tags of playlist. */
        public void saveTags(String s, Vector<Object> rows) {
            Vector<File> r = (Vector<File>)rows.clone();
            playlistTags.put(s, r);
        }

        /** Get tags of some playlist. */
        public Object getPlaylistTags(String s) {
            currentPlaylist = s;
            return playlistTags.get(s);
        }

        /** Get location of mediafolder. */
        public File getMediaFolder() {
            return mediaFolder;
        }

        /** Set current playlist. */
        public void setCurrPlayList(String s) {
            currentPlaylist = s;

            setChanged();
            notifyObservers("switchPl");
        }

        /** Add song to playlist. */
        public void addSong(File[] newSong) {
            Vector<File> vf = getPlayList(currentPlaylist);
            for(File song : newSong) {
                for(File songM : mediaFolderContentF) {
                    if(song.equals(songM)) {
                        song = songM;
                        vf.add(song);
                    }
                }
            }

            setChanged();
            notifyObservers("addFile");
        }

        /** Remove song from playlist. */
        public void removeSong(int selectedRows[]) {
            for(int i=selectedRows.length-1;i>-1;i--) {
                ((Vector<File>)playlistList.get(currentPlaylist)).remove(selectedRows[i]);
                ((Vector<Object>)playlistTags.get(currentPlaylist)).remove(selectedRows[i]);
            }

            setChanged();
            notifyObservers("delete");
        }

        /** Remove playlist. */
        public void removePl(String s) {
            playlistList.remove(s);
            playlistTags.remove(s);

            setChanged();
            notifyObservers("removePl");
        }

        /** Save playlist. */
        public void savePl(String s) throws Exception {
            serialize((Vector<File>)playlistList.get(s), s);
            serialize((Vector<File>)playlistTags.get(s), s + ".tg");

            setChanged();
            notifyObservers("savePl");
        }

        /** Load playlist. */
        public void loadPl(String s)  throws Exception {
            Vector<File> pl = new Vector<File>();
            Vector<Object> plTags = new Vector<Object>();
            pl = (Vector<File>)deserialize(s);
            plTags = (Vector<Object>)deserialize(s + ".tg");
            playlistList.put(s, pl);
            playlistTags.put(s, plTags);


            for(int i=0;i<pl.size();i++) {
                Vector<Object> f1 = (Vector<Object>)plTags.get(i);
                File s1 = mediaFolderContentF.get(mediaFolderContentF.indexOf(pl.get(i)));
                Tags.readSingleTag(f1, s1);
            }

            setChanged();
            notifyObservers(s);
        }

        /** Clear playlist. */
        public void clearPl() {
            Vector<Object> f = (Vector<Object>)(playlistList.get(currentPlaylist));
            if(!f.isEmpty()) {
                f.clear();
                f = (Vector<Object>)playlistTags.get(currentPlaylist);
                f.clear();
            }

            setChanged();
            notifyObservers("clearPl");
        }

        /** Move song up one position. */
        public void up(int i) {
            if(i+1 != 0) {
                Vector<Object> vo = (Vector<Object>)playlistList.get(currentPlaylist);
                Object o = vo.get(i);

                vo.set(i, vo.get(i+1));
                vo.set(i+1, o);

                setChanged();
                notifyObservers("up");
            }
        }

        /** Move song down one position. */
        public void down(int i) {
            Vector<Object> vo = (Vector<Object>)playlistList.get(currentPlaylist);

            if(i < vo.size()) {
                Object o = vo.get(i);

                vo.set(i, vo.get(i-1));
                vo.set(i-1, o);

                setChanged();
                notifyObservers("down");
            }
        }
}
