import java.util.*;

public class PriorityPreemptiveScheduler {

    public static List<String> prioritySchedulingPreemptive(List<Process> processes) {
        List<String> output = new ArrayList<>();
        int currentTime = 0;
        int totalWaitingTime = 0;

        output.add("PR_withPREMP");
        processes.sort(Comparator.comparingInt(p -> p.arrivalTime));
        PriorityQueue<Process> readyQueue = new PriorityQueue<>(Comparator.comparingInt(p -> p.priority));
        int index = 0;

        while (index < processes.size() || !readyQueue.isEmpty()) {
            while (index < processes.size() && processes.get(index).arrivalTime <= currentTime) {
                readyQueue.add(processes.get(index));
                index++;
            }

            if (readyQueue.isEmpty()) {
                currentTime = processes.get(index).arrivalTime;
                continue;
            }

            Process currentProcess = readyQueue.poll();
            output.add(currentTime + "\t" + currentProcess.processID);

            int nextEventTime = (index < processes.size()) ? processes.get(index).arrivalTime : Integer.MAX_VALUE;
            int timeSlice = Math.min(currentProcess.remainingTime, nextEventTime - currentTime);

            currentProcess.remainingTime -= timeSlice;
            currentTime += timeSlice;

            if (currentProcess.remainingTime > 0) {
                readyQueue.add(currentProcess);
            } else {
                int waitingTime = currentTime - currentProcess.arrivalTime - currentProcess.burstTime;
                totalWaitingTime += waitingTime;
            }
        }

        double averageWaitingTime = (double) totalWaitingTime / processes.size();
        output.add(String.format("AVG Waiting Time: %.2f", averageWaitingTime));

        return output;
    }

    // Extra Credit: Binary Heap implementation
    public static List<String> prioritySchedulingPreemptiveBinaryHeap(List<Process> processes) {
        List<String> output = new ArrayList<>();
        int currentTime = 0;
        int totalWaitingTime = 0;

        output.add("PR_withPREMP (Binary Heap)");
        processes.sort(Comparator.comparingInt(p -> p.arrivalTime));
        PriorityQueue<Process> binaryHeap = new PriorityQueue<>(Comparator.comparingInt(p -> p.priority));
        int index = 0;

        while (index < processes.size() || !binaryHeap.isEmpty()) {
            while (index < processes.size() && processes.get(index).arrivalTime <= currentTime) {
                binaryHeap.add(processes.get(index));
                index++;
            }

            if (binaryHeap.isEmpty()) {
                currentTime = processes.get(index).arrivalTime;
                continue;
            }

            Process currentProcess = binaryHeap.poll();
            output.add(currentTime + "\t" + currentProcess.processID);

            int nextEventTime = (index < processes.size()) ? processes.get(index).arrivalTime : Integer.MAX_VALUE;
            int timeSlice = Math.min(currentProcess.remainingTime, nextEventTime - currentTime);

            currentProcess.remainingTime -= timeSlice;
            currentTime += timeSlice;

            if (currentProcess.remainingTime > 0) {
                binaryHeap.add(currentProcess);
            } else {
                int waitingTime = currentTime - currentProcess.arrivalTime - currentProcess.burstTime;
                totalWaitingTime += waitingTime;
            }
        }

        double averageWaitingTime = (double) totalWaitingTime / processes.size();
        output.add(String.format("AVG Waiting Time: %.2f", averageWaitingTime));

        return output;
    }

    // Extra Credit: Unsorted Array implementation
    public static List<String> prioritySchedulingPreemptiveUnsortedArray(List<Process> processes) {
        List<String> output = new ArrayList<>();
        int currentTime = 0;
        int totalWaitingTime = 0;

        output.add("PR_withPREMP (Unsorted Array)");
        processes.sort(Comparator.comparingInt(p -> p.arrivalTime));
        List<Process> unsortedList = new ArrayList<>();
        int index = 0;

        while (index < processes.size() || !unsortedList.isEmpty()) {
            while (index < processes.size() && processes.get(index).arrivalTime <= currentTime) {
                unsortedList.add(processes.get(index));
                index++;
            }

            if (unsortedList.isEmpty()) {
                currentTime = processes.get(index).arrivalTime;
                continue;
            }

            // Find the process with the highest priority (smallest priority number)
            Process currentProcess = unsortedList.stream().min(Comparator.comparingInt(p -> p.priority)).orElseThrow();
            unsortedList.remove(currentProcess);

            output.add(currentTime + "\t" + currentProcess.processID);

            int nextEventTime = (index < processes.size()) ? processes.get(index).arrivalTime : Integer.MAX_VALUE;
            int timeSlice = Math.min(currentProcess.remainingTime, nextEventTime - currentTime);

            currentProcess.remainingTime -= timeSlice;
            currentTime += timeSlice;

            if (currentProcess.remainingTime > 0) {
                unsortedList.add(currentProcess);
            } else {
                int waitingTime = currentTime - currentProcess.arrivalTime - currentProcess.burstTime;
                totalWaitingTime += waitingTime;
            }
        }

        double averageWaitingTime = (double) totalWaitingTime / processes.size();
        output.add(String.format("AVG Waiting Time: %.2f", averageWaitingTime));

        return output;
    }
}
