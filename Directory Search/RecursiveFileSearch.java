import java.io.File;
import java.util.ArrayList;
import java.util.List;

public class RecursiveFileSearch {

    public static List<String> paths = new ArrayList<>();

    public static void searchForFileOrDir(File directory, String fileOrDirName) {
        if (directory.isDirectory()) {
            if (directory.getName().equals(fileOrDirName)) {
                String filePath = directory.getAbsolutePath().toString();
                paths.add(filePath);
            }
            System.out.println("Searching in : " + directory.toString());
            if (directory.canRead()) {
                for (File nextDir : directory.listFiles()) {
                    if (nextDir.isDirectory()) {
                        searchForFileOrDir(nextDir, fileOrDirName);
                    } else {
                        if (nextDir.getName().equals(fileOrDirName)) {
                            String filePath = nextDir.getAbsolutePath().toString();
                            paths.add(filePath);
                        }
                    }
                }
            }
        }
    }

    public static void main(String[] args) {
        String fileName = args[0];
        String currentDir = System.getProperty("user.dir");
        searchForFileOrDir(new File(currentDir), fileName);
        if (paths.size() != 0) {
            System.out.println("Found: ");
            for (String path : paths) {
                System.out.println(path);
            }
        } else {
            System.out.println("Not found!");
        }
    }
}
