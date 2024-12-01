/* 
Ryan Sario
CSC139-01 

How to run: In terminal > navigate to assignment's parent directory > 
            javac Assignment4/VirtualMemorySimulator.java >
            java Assignment4.VirtualMemorySimulator
*/

package Assignment4;

import java.io.*;
import java.util.*;

public class VirtualMemorySimulator {
    public static void main(String[] args) {
        String inputFolder = "Assignment4/inputs";
        String outputFolder = "Assignment4/outputs";

        File inputDir = new File(inputFolder);
        File outputDir = new File(outputFolder);

        // Create output directory if it doesn't exist
        if (!outputDir.exists()) {
            outputDir.mkdir();
        }

        // Process each input file in the input folder
        File[] inputFiles = inputDir.listFiles((dir, name) -> name.endsWith(".txt"));
        if (inputFiles == null || inputFiles.length == 0) {
            System.err.println("No input files found in " + inputFolder);
            return;
        }

        for (File inputFile : inputFiles) {
            try {
                // Read input file
                BufferedReader reader = new BufferedReader(new FileReader(inputFile));
                String outputFilePath = outputFolder + "/" + inputFile.getName().replace(".txt", "_output.txt");
                PrintWriter writer = new PrintWriter(new FileWriter(outputFilePath));

                // Read the first line for configuration
                String[] config = reader.readLine().split("\\s+");
                int numPages = Integer.parseInt(config[0]);
                int numFrames = Integer.parseInt(config[1]);
                int numRequests = Integer.parseInt(config[2]);

                // Read the page requests
                List<Integer> pageRequests = new ArrayList<>();
                for (int i = 0; i < numRequests; i++) {
                    pageRequests.add(Integer.parseInt(reader.readLine().trim()));
                }
                reader.close();

                // Execute and write results
                writer.println(String.join("\n", simulateFIFO(pageRequests, numFrames)));
                writer.println();
                writer.println(String.join("\n", simulateOptimal(pageRequests, numFrames)));
                writer.println();
                writer.println(String.join("\n", simulateLRU(pageRequests, numFrames)));
                writer.close();

                System.out.println("Processed " + inputFile.getName() + " -> " + outputFilePath);
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    private static List<String> simulateFIFO(List<Integer> pageRequests, int frames) {
        List<String> result = new ArrayList<>();
        result.add("FIFO");
        Queue<Integer> frameQueue = new LinkedList<>();
        Map<Integer, Integer> frameMap = new HashMap<>(); // Map page to frame index
        int pageFaults = 0;
        int frameIndex = 0;

        for (int page : pageRequests) {
            if (!frameMap.containsKey(page)) {
                pageFaults++;
                if (frameQueue.size() == frames) {
                    int removed = frameQueue.poll();
                    int removedIndex = frameMap.remove(removed);
                    result.add("Page " + removed + " unloaded from Frame " + removedIndex + ", Page " + page + " loaded into Frame " + removedIndex);
                    frameQueue.add(page);
                    frameMap.put(page, removedIndex);
                } else {
                    frameQueue.add(page);
                    frameMap.put(page, frameIndex);
                    result.add("Page " + page + " loaded into Frame " + frameIndex);
                    frameIndex++;
                }
            } else {
                result.add("Page " + page + " already in Frame " + frameMap.get(page));
            }
        }
        result.add(pageFaults + " page faults");
        return result;
    }

    private static List<String> simulateOptimal(List<Integer> pageRequests, int frames) {
        List<String> result = new ArrayList<>();
        result.add("Optimal");
        List<Integer> frameList = new ArrayList<>();
        Map<Integer, Integer> frameMap = new HashMap<>();
        int pageFaults = 0;

        for (int i = 0; i < pageRequests.size(); i++) {
            int page = pageRequests.get(i);
            if (!frameMap.containsKey(page)) {
                pageFaults++;
                if (frameList.size() == frames) {
                    int toRemove = findOptimalReplacement(frameList, pageRequests, i);
                    int removed = frameList.get(toRemove);
                    result.add("Page " + removed + " unloaded from Frame " + toRemove + ", Page " + page + " loaded into Frame " + toRemove);
                    frameList.set(toRemove, page);
                    frameMap.remove(removed);
                    frameMap.put(page, toRemove);
                } else {
                    frameList.add(page);
                    frameMap.put(page, frameList.size() - 1);
                    result.add("Page " + page + " loaded into Frame " + (frameList.size() - 1));
                }
            } else {
                result.add("Page " + page + " already in Frame " + frameMap.get(page));
            }
        }
        result.add(pageFaults + " page faults");
        return result;
    }

    private static int findOptimalReplacement(List<Integer> frameList, List<Integer> pageRequests, int currentIndex) {
        Map<Integer, Integer> nextUse = new HashMap<>();
        for (int page : frameList) {
            int nextIndex = Integer.MAX_VALUE;
            for (int j = currentIndex + 1; j < pageRequests.size(); j++) {
                if (pageRequests.get(j) == page) {
                    nextIndex = j;
                    break;
                }
            }
            nextUse.put(page, nextIndex);
        }
        return frameList.indexOf(Collections.max(nextUse.entrySet(), Map.Entry.comparingByValue()).getKey());
    }

    private static List<String> simulateLRU(List<Integer> pageRequests, int frames) {
        List<String> result = new ArrayList<>();
        result.add("LRU");
        List<Integer> frameList = new ArrayList<>();
        Map<Integer, Integer> lastUsed = new HashMap<>();
        int pageFaults = 0;

        for (int i = 0; i < pageRequests.size(); i++) {
            int page = pageRequests.get(i);
            if (!frameList.contains(page)) {
                pageFaults++;
                if (frameList.size() == frames) {
                    int toRemove = findLeastRecentlyUsed(frameList, lastUsed);
                    int removed = frameList.get(toRemove);
                    result.add("Page " + removed + " unloaded from Frame " + toRemove + ", Page " + page + " loaded into Frame " + toRemove);
                    frameList.set(toRemove, page);
                } else {
                    frameList.add(page);
                    result.add("Page " + page + " loaded into Frame " + (frameList.size() - 1));
                }
            } else {
                result.add("Page " + page + " already in Frame " + frameList.indexOf(page));
            }
            lastUsed.put(page, i);
        }
        result.add(pageFaults + " page faults");
        return result;
    }

    private static int findLeastRecentlyUsed(List<Integer> frameList, Map<Integer, Integer> lastUsed) {
        return frameList.indexOf(Collections.min(frameList, Comparator.comparingInt(lastUsed::get)));
    }
}
