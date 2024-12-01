import java.util.*;

public class RoundRobinScheduler {

    public static List<String> roundRobinScheduling(List<Process> processes, int timeQuantum) {
        Queue<Process> readyQueue = new LinkedList<>();
        List<String> output = new ArrayList<>();
        int currentTime = 0;
        int totalWaitingTime = 0;

        output.add("RR " + timeQuantum);
        processes.sort(Comparator.comparingInt(p -> p.arrivalTime));
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

            int timeSlice = Math.min(currentProcess.remainingTime, timeQuantum);
            currentProcess.remainingTime -= timeSlice;
            currentTime += timeSlice;

            for (Process p : readyQueue) {
                totalWaitingTime += timeSlice;
            }

            if (currentProcess.remainingTime > 0) {
                readyQueue.add(currentProcess);
            }
        }

        double averageWaitingTime = (double) totalWaitingTime / processes.size();
        output.add(String.format("AVG Waiting Time: %.2f", averageWaitingTime));

        return output;
    }
}
