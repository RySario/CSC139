public class Process {
    public int processID;
    public int arrivalTime;
    public int burstTime;
    public int priority;
    public int remainingTime;
    public int completionTime;

    public Process(int processID, int arrivalTime, int burstTime, int priority) {
        this.processID = processID;
        this.arrivalTime = arrivalTime;
        this.burstTime = burstTime;
        this.priority = priority;
        this.remainingTime = burstTime;
        this.completionTime = 0;
    }
}
