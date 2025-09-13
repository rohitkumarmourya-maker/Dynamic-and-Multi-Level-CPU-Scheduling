# Operating Systems Lab – Assignment IV  
## Dynamic and Multi-Level CPU Scheduling  

### Student Information
- **Course**: CSL3030 – Operating Systems Lab  
- **Institute**: Indian Institute of Technology, Jodhpur  
- **Assignment**: Lab IV – Dynamic and Multi-Level CPU Scheduling  
- **Tasks Implemented**:  
  1. Anti-Starvation Priority Scheduler (Preemptive Priority Scheduling with Aging)  
  2. Multi-Level Queue Scheduler (Foreground RR + Background FCFS)  

---

## 📌 Objective
The goal of this assignment is to implement **advanced CPU scheduling algorithms** that address real-world challenges like **process starvation** and the **need to prioritize interactive jobs** over background tasks. Both implementations account for:
- **I/O bursts**
- **Non-zero context switching overhead**
- **Detailed Gantt charts**
- **Process statistics** (Waiting Time, Turnaround Time, Response Time, CPU Utilization)

---

## ⚙️ Task 1: Anti-Starvation Priority Scheduler
### Problem It Solves
In **basic preemptive priority scheduling**, lower-priority processes may starve if higher-priority jobs keep arriving. To prevent starvation, **aging** is introduced.

### Key Features
- Preemptive priority scheduling.  
- **Aging mechanism**: For every `X` ms spent in the ready queue, a process’s priority improves (numerical priority decreases).  
- Maintains **ready queue** and **blocked queue** (for I/O).  
- Produces a Gantt chart with **CPU bursts, I/O waits, and context switches**.  
- Calculates and displays WT, TAT, RT, and CPU Utilization.  

### When to Use
This scheduler is best when:  
- **Fairness is critical**, and no process should wait indefinitely.  
- Systems where **starvation risk is high** (e.g., batch systems with mixed priority jobs).  

---

## ⚙️ Task 2: Multi-Level Queue Scheduler
### Problem It Solves
Different jobs have different scheduling needs.  
- **Foreground (interactive) jobs** need fast response times.  
- **Background (batch) jobs** can tolerate delays but must eventually complete.  

### Key Features
- Two fixed queues:  
  - **Foreground (FG)**: Scheduled using **Round Robin (RR)** with a time quantum.  
  - **Background (BG)**: Scheduled using **First-Come, First-Serve (FCFS)**.  
- **Absolute priority** to Foreground queue. Background processes only execute if FG is empty.  
- FCFS processes can be **preempted** if a new FG process arrives.  
- Unified Gantt chart and process statistics.  

### When to Use
This scheduler is best when:  
- **Interactive applications** (user-facing tasks) must be prioritized.  
- Systems require **clear separation between FG and BG workloads** (e.g., desktops, interactive servers).  

---

## 🔍 Comparative Analysis

| Feature | Anti-Starvation Priority | Multi-Level Queue |
|---------|--------------------------|------------------|
| **Problem Solved** | Prevents starvation of low-priority processes | Ensures interactive tasks are prioritized over batch tasks |
| **Mechanism** | Aging improves priority over time | Foreground RR queue has absolute priority over Background FCFS |
| **Fairness** | High (all processes eventually execute) | Fair within each queue, but BG jobs may wait long |
| **Responsiveness** | Moderate | High for FG jobs, low for BG jobs |
| **Use Case** | Batch systems needing fairness | Interactive systems needing fast response |

---

## 📊 Example Output
### Task 1 (Anti-Starvation Priority Scheduler)
- Gantt chart shows **priority-based preemption** with context switches.  
- Average Waiting Time: ~13.6  
- CPU Utilization: ~68.75%  

### Task 2 (Multi-Level Queue Scheduler)
- Gantt chart shows **RR for FG** and **FCFS for BG** with preemptions.  
- Average Waiting Time: ~17.6  
- CPU Utilization: ~64.1%  

---

## ▶️ How to Run
1. Compile the program:  
   ```bash
   g++ Main.cpp -o scheduler
