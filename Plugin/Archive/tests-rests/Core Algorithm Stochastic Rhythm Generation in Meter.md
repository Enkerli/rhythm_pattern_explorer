**🧠 Core Algorithm: Stochastic Rhythm Generation in Meter**

**Implemented in**: *kin.rhythmicator* (Max/MSP & Max4Live device)

**Goal**: Generate rhythm patterns in real time, controllable via high-level musical parameters

**Main Inputs**:
- Meter (e.g. 3/4, 6/8)
- Metrical subdivision level (e.g. 16th notes)
- User-controlled parameters: *density*, *syncopation*, *metrical strength*, *variation*, *event articulation*

---

### **1.**

### **Phase 1: Weight Assignment via Stratified Meter + Barlow’s Indispensability**
1. **Stratify Meter**
   - Decompose number of pulses into prime factors.
   - Example: 3/4 meter with 16th notes → 12 pulses → factors: 1×3×2×2 (i.e., bar → 3 quarters → 6 eighths → 12 sixteenths)
2. **Barlow’s Indispensability Algorithm**
   - Assign *importance values* to each pulse across stratification levels.
   - Formula incorporates:
     - Prime stratification
     - Relative strength within each prime subdivision
     - Multiplicative weighting across levels
     - Left/right rotations for final weighting
   - Result: A single numerical weight for each pulse, reflecting how *indispensable* it is to the meter.
3. **Assign Pulse Weights**
   - For each stratification level i, assign weights from a range Wi:

     W_i = [R^{i-1}, R^i)

     where R ∈ [0,1] controls **density**:
     - R = 0: Only top-level strong beats get weight
     - R = 1: All pulses weighted equally

---

### **2.**

### **Phase 2: Stochastic Performance Generation**
1. **Triggering Events**
   - At each pulse, an event is triggered with probability:

     λ = \frac{W_\ell \cdot n}{M + ε}

     where:
     - Wℓ is weight of pulse ℓ
     - M is **metrical strength** parameter
     - ε prevents zero division if Wℓ = 0
2. **Amplitude Scaling**
   - Amplitudes are directly proportional to weights (at R = 0.5)
   - Emphasizes stronger beats

---

### **3.**

### **Syncopation Algorithm**
- Introduces anticipation: trigger note at weak pulse *before* a strong one
- Controlled by PS (syncopation probability)
- Two **musicality restrictions**:
  1. Too many syncopated pulses get automatically unsyncopated.
  2. Prevents double-hits: mutes a strong beat if the previous beat was syncopated.

---

### **4.**

### **Variation Algorithm**

Two Modes:
- **Stable**: Varies around initial pattern
- **Unstable**: Random walk away from initial pattern

Variation is implemented by selectively allowing event/syncopation decisions to change across cycles.

---

### **5.**

### **Complexity Space**

A 2D user interface controlling:
- **Metrical strength** (M)
- **Syncopation** (PS)
- **Variation amount**

This defines a **“rhythmic complexity” space** where distance from center increases perceived rhythmic complexity.

---

### **Summary: User-Controllable Parameters**

**Parameter**

**Controls**

R

Density of events

M

Strength of meter (probability shaping)

PS

Amount of syncopation

Variation Mode

Stability of pattern

Articulation

Staccato vs Legato

Complexity Vector

Maps multiple parameters in 2D