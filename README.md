**Overview:**

This coursework implements an adaptive aggregation algorithm to process sensor data based on activity levels, balancing detail preservation during high activity and resource efficiency during low activity.

---

**Goals:**
- Data Storage: Read and store light sensor data in a buffer (12 readings, 2 readings/second).

- Activity Measurement: Use standard deviation to classify activity levels (low, moderate, high).

- Data Aggregation:
  - Low activity: Aggregate 12 readings into 1 value.
  - Moderate activity: Aggregate 4 readings into 1 value.
  - High activity: No aggregation.

- Reporting: Display results in the terminal.

---

**Deliverables:**

- Source code implementing the algorithm (must run within Contiki OS).
- A PDF containing screenshots of: high activity (no aggregation); moderate activity (4-into-1 aggregation) and low activity (12-into-1 aggregation).
