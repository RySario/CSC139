import java.util.*;

public class PriorityScheduler {

    public static List<String> priorityScheduling(List<Process> processes) {
        List<String> output = new ArrayList<>();
        int currentTime = 0;
        int totalWaitingTime = 0;

        output.add("PR_noPREMP");
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

            currentTime += currentProcess.burstTime;
            int waitingTime = currentTime - currentProcess.arrivalTime - currentProcess.burstTime;
            totalWaitingTime += waitingTime;
        }

        double averageWaitingTime = (double) totalWaitingTime / processes.size();
        output.add(String.format("AVG Waiting Time: %.2f", averageWaitingTime));

        return output;
    }
}
