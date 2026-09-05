# Touchless Washbasin Sensors 🚰

The goal was to create a modern replacement for legacy washbasin sensors: the older models were built around obsolete components that have since been discontinued, becoming rare and overpriced.

After testing numerous alternatives, the top options that proved best suited for high-humidity environments and real-world user interaction are:

1. **HLK-LD2416-based sensor**
2. **HLK-LD1020 + CH32V003 sensor**
3. **ToF400 / ToF200 + CH32V003 sensor**

---

### Comparison & Personal Experience

* **HLK-LD2416 (Most optimal version):**  
  I consider this the best overall choice because the sensor practically does not require an external microcontroller and can be easily configured via a standard USB-to-UART converter.  
  *(Yes, there is undeniable comedic irony in taking an advanced 24 GHz FMCW millimeter-wave radar packed with micro-motion tracking and distance-gating capabilities, only to reduce its life's purpose to the gloriously primitive task of opening a water tap when someone wants to wash their hands. It’s total overkill on paper, but in practice, it works brilliantly.)*

* **HLK-LD1020 + CH32V003:**  
  I also really like this version. In terms of raw component cost, it turns out slightly cheaper than the LD2416. However, it takes significantly longer and is much harder to assemble by hand, while objectively, there is practically no difference in real-world performance.

* **ToF400 / ToF200 + CH32V003:**  
  My least favorite. The sensor is relatively expensive and significantly compromises the enclosure's sealing and water resistance because it requires an open line of sight for the optical lens. Radar-based modules, on the other hand, detect movement directly through solid plastic, which is a massive advantage for complete waterproof sealing in plumbing fixtures.

---

### Special Mention: RCWL-0516 (Why it was an absolute failure)

While radar technology is the best direction for this application, definitely not every radar module is up to the task. Throughout the project, I went through tons of different sensors and parts, but I specifically want to single out the **RCWL-0516** because of just how exceptionally bad it turned out to be.

Even though many people online claim they work fine, in my actual experience with a batch of 20 units:
* About half were completely dead or half-broken right out of the box;
* Another portion had a detection range of barely half a meter;
* Only a tiny fraction behaved relatively predictably.

Between the massive defect rate, bulky footprint, and outdated design, out of everything I tested, this one stands out as completely unusable for any reliable hardware build.

---

### CH32V003 Microcontroller (SOP-8)

For radar versions that require auxiliary logic, the **CH32V003 in an SOP-8 package** was used. 

It is an ideal fit for this kind of project thanks to:
* Extremely low unit cost;
* Ease of hand-soldering with a standard soldering iron;
* Simple, straightforward firmware development.





### WORK LD2416-based sensor

https://github.com/user-attachments/assets/3fb2ff55-c5b9-49b8-b7ec-d0e878508fb5


