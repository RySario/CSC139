import java.io.*;
import java.util.*;

public class Main {

    public static void main(String[] args) {
        File testFolder = new File("test_cases");
        File[] files = testFolder.listFiles((dir, name) -> name.startsWith("input") && name.endsWith(".txt"));

        if (files != null) {
            for (File inputFile : files) {
                String inputFileName = inputFile.getName();
                String testNumber = inputFileName.replace("input", "").replace(".txt", "");
                File expectedOutputFile = new File("test_cases/output" + testNumber + ".txt");

                System.out.println("Running test case: " + inputFileName);

                try {
                    List<Process> processes = loadProcesses(inputFile);
                    String algorithmType = identifyAlgorithmType(inputFile);

                    List<String> actualOutput;
                    if (algorithmType.equals("RR")) {
                        int timeQuantum = extractTimeQuantum(inputFile);
                        actualOutput = RoundRobinScheduler.roundRobinScheduling(processes, timeQuantum);
                    } else if (algorithmType.equals("SJF")) {
                        actualOutput = ShortestJobFirstScheduler.shortestJobFirstScheduling(processes);
                    } else if (algorithmType.equals("PR_noPREMP")) {
                        actualOutput = PriorityScheduler.priorityScheduling(processes);
                    } else if (algorithmType.equals("PR_withPREMP")) {
                        actualOutput = PriorityPreemptiveScheduler.prioritySchedulingPreemptive(processes);
                    } else {
                        throw new IllegalArgumentException("Unknown algorithm type in " + inputFileName);
                    }

                    compareOutput(expectedOutputFile, actualOutput, testNumber);

                } catch (IOException e) {
                    System.out.println("Error reading files for test " + testNumber);
                    e.printStackTrace();
                }
            }
        } else {
            System.out.println("No test cases found in the test_cases folder.");
        }

        // Run extra credit timing comparison
        runExtraCreditTiming();
    }

    private static void runExtraCreditTiming() {
        System.out.println("\n=== Extra Credit: Timing Comparison ===");
        File inputFile = new File("test_cases/input16.txt");

        try {
            List<Process> processes = loadProcesses(inputFile);

            // Binary Heap implementation
            System.out.println("Binary Heap Implementation:");
            long startBinaryHeap = System.nanoTime();
            List<String> binaryHeapOutput = PriorityPreemptiveScheduler.prioritySchedulingPreemptiveBinaryHeap(new ArrayList<>(processes));
            long endBinaryHeap = System.nanoTime();
            binaryHeapOutput.forEach(System.out::println);
            System.out.printf("Execution Time (Binary Heap): %.2f ms%n%n", (endBinaryHeap - startBinaryHeap) / 1e6);

            // Unsorted Array implementation
            System.out.println("Unsorted Array Implementation:");
            long startUnsortedArray = System.nanoTime();
            List<String> unsortedArrayOutput = PriorityPreemptiveScheduler.prioritySchedulingPreemptiveUnsortedArray(new ArrayList<>(processes));
            long endUnsortedArray = System.nanoTime();
            unsortedArrayOutput.forEach(System.out::println);
            System.out.printf("Execution Time (Unsorted Array): %.2f ms%n%n", (endUnsortedArray - startUnsortedArray) / 1e6);

        } catch (IOException e) {
            System.out.println("Error running extra credit timing.");
            e.printStackTrace();
        }
    }

    private static void compareOutput(File expectedOutputFile, List<String> actualOutput, String testNumber) throws IOException {
        List<String> expectedOutput = readExpectedOutput(expectedOutputFile);

        // Display test case result
        if (actualOutput.equals(expectedOutput)) {
            System.out.println("Test " + testNumber + " PASSED");
        } else {
            System.out.println("Test " + testNumber + " FAILED");
        }

        // Print both expected and actual output for every test case
        System.out.println("Expected Output:");
        expectedOutput.forEach(System.out::println);
        System.out.println("Actual Output:");
        actualOutput.forEach(System.out::println);
        System.out.println();
    }

    private static List<Process> loadProcesses(File inputFile) throws IOException {
        BufferedReader reader = new BufferedReader(new FileReader(inputFile));
        List<Process> processes = new ArrayList<>();

        String algorithm = reader.readLine();
        int numProcesses = Integer.parseInt(reader.readLine().trim());

        for (int i = 0; i < numProcesses; i++) {
            String[] processDetails = reader.readLine().split(" ");
            int id = Integer.parseInt(processDetails[0]);
            int arrival = Integer.parseInt(processDetails[1]);
            int burst = Integer.parseInt(processDetails[2]);
            int priority = Integer.parseInt(processDetails[3]);
            processes.add(new Process(id, arrival, burst, priority));
        }

        reader.close();
        return processes;
    }

    private static String identifyAlgorithmType(File inputFile) throws IOException {
        BufferedReader reader = new BufferedReader(new FileReader(inputFile));
        String algorithm = reader.readLine().split(" ")[0];
        reader.close();
        return algorithm;
    }

    private static int extractTimeQuantum(File inputFile) throws IOException {
        BufferedReader reader = new BufferedReader(new FileReader(inputFile));
        String line = reader.readLine();
        int timeQuantum = Integer.parseInt(line.split(" ")[1]);
        reader.close();
        return timeQuantum;
    }

    private static List<String> readExpectedOutput(File expectedOutputFile) throws IOException {
        List<String> output = new ArrayList<>();
        BufferedReader reader = new BufferedReader(new FileReader(expectedOutputFile));
        String line;
        while ((line = reader.readLine()) != null) {
            output.add(line);
        }
        reader.close();
        return output;
    }
}
