package kickassplayer;

import java.io.*;
import javax.sound.sampled.*;
import java.util.Vector;

/**
* Player class.
* <p>
* Course work. GRTOT. Spring 2010.
* <p>
* Modified 20.03.2010
*
* @author Nikita Ishkov (nikita.ishkov@uta.fi)
*/
public class MP3Player extends Thread {

    private AudioInputStream audioInputStream;
    private File input;
    private static SourceDataLine line;

    private Controller controller;

    private boolean quit;
    private boolean playOnlyOnce;
    private static FloatControl volume;
    private static float currVolume;
    private Model model;

    public MP3Player(Controller c, Model m, boolean playOnce) {
        super("player");
        controller = c;
        model = m;
        playOnlyOnce = playOnce;
    }

    public void setFile(File i) {
        if (i != null)
            input = i;
    }

    @Override
    public void run() {
        play(input);
    }

    public void stopPlay() {
        if (line != null) {
            try {
                quit = true;
                line.stop();
                line.close();
                audioInputStream.close();
            }
            catch(Exception e) {}
        }
    }

    public void stopAll() {
        if (line != null) {
            try {
                line.drain();
                line.stop();
                line.close();
                audioInputStream.close();
            }
            catch(Exception e) {}
        }
    }

    public void play(File f) {
        try {

            if (line != null)
                stopPlay();

            input = f;

            AudioInputStream in = AudioSystem.getAudioInputStream(input);

            AudioFormat baseFormat = in.getFormat();
            AudioFormat decodedFormat = new AudioFormat(
                    AudioFormat.Encoding.PCM_SIGNED, // Encoding to use
                    baseFormat.getSampleRate(),	  // sample rate (same as base format)
                    16,				  // sample size in bits (thx to Javazoom)
                    baseFormat.getChannels(),	  // # of Channels
                    baseFormat.getChannels()*2,	  // Frame Size
                    baseFormat.getSampleRate(),	  // Frame Rate
                    false                         // Big Endian
            );

            audioInputStream = AudioSystem.getAudioInputStream(decodedFormat, in);

            DataLine.Info info = new DataLine.Info(SourceDataLine.class, decodedFormat);
            line = (SourceDataLine) AudioSystem.getLine(info);

            quit = false;

            if(line != null) {
                line.open(decodedFormat);
                byte[] data = new byte[4096];

                // show volume volume
                volume = (FloatControl) line.getControl( FloatControl.Type.MASTER_GAIN );
                volume.setValue(currVolume);

                // Start
                line.start();
                controller.updatePlayingInfo();

                long sec = 0;
                float length = 0;
                int nBytesRead = 0;
                Vector<Object> vo = (Vector<Object>)model.getSongTags(input);
                String[] time = vo.get(5).toString().split(":");

                length = Integer.parseInt(time[0])*60 + Integer.parseInt(time[1]);

                System.err.println(length);

                while ((nBytesRead = audioInputStream.read(data, 0, data.length)) != -1) {
                    line.write(data, 0, nBytesRead);

                    // show progress bar
                    sec = line.getMicrosecondPosition()/1000000;
                    Controls.progress.setValue((int)((100/length)*sec));
                    
                    if(quit) {
                        line.stop();
                        line.flush();
                    }
                }
                
                // Stop
                line.drain();
                line.stop();
                line.close();
                audioInputStream.close();

                // Continue playing if not forced to close
                if (!quit && !playOnlyOnce)
                    controller.playForward();
                else
                    controller.playingStopped();

            }

        }
        catch (Exception e) {
            System.err.println(e.toString());
        }
        finally {
            if(audioInputStream != null) {
                    try { audioInputStream.close(); }
                    catch(IOException e) { }
            }
        }
    }

    public static void setVolume(Integer fps) {
        currVolume = fps;

        if(line != null)
            volume.setValue((float)fps);
    }

    public void setPlayOnce(boolean b) {
        playOnlyOnce = b;
    }
}

