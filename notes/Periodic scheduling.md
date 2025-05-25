
What is the worst case situation with fixed priority?
- when tasks arrive simultanously, it always is the worst case condition.


The highest priority does not suffer inteference form the lower priority ones.

The lower priority, suffer from the higher one.

Pushing all tasks to arrive at the same time, is the one who causes alla largest task times.


Sporadic tasks --> arrive spaced by at least one period. It cannot be less than one period

**modello sporadico** : t_i è il minimo tempo di arrivo.
- due job dello stesso task arrivano almeno con lo stesso tempo di t.

I task sporadici sono almeno periodici, arrivano dopo un periodo o anche più tardi, il caso peggiore è quando arrivano tutti assieme e strettamente periodici.
- il test che ho usato prima per Ulb holds identically

T_i è il minimo tempo di interarrivo. ***Sporadic is identical from a scheduling point of view***

![[Recording 20250513180405.m4a]]



**In which sense is RM optimal?**
se non riesco a schedulare il mio task system, allora nessun altro mi permetterà di schedularlo tranquillamente. 
- se non ce la fa RM, non ce la fa nessun altro. 

![[Recording 20250513180705.m4a]]



Slide 33 da skippare.
- è un'altra condizione sufficiente, è tighter, più stretta del Ulb, porta Ulb più su del 69%.

Riesci ad allargare la regione sopra la quale riesci a schedulare-


# Dynamic priority scheduling

EDF --> earliest deadline tasks, l'avevamo visto
- schedulava in base alla deadline assoluta. The task that has the earliest deadline has bigger priority

Ci interessa ora il **tesk della priorità** quando la priorità è dinamica.

$$ d_{i,k} = r_{i,k} + D_i $$
Don't study the proof


HOW DOES EDF Differ from RM when taking a schedule decisione


![[Recording 20250513181344.m4a]]


**EDF vs RM when scheduling** slide 38
which task has an earlier deadline 
--> always give priority to **the executing one with the earliest deadline**

***EDF is optimal amongst all of the algorithms***.


![[Recording 20250513181607.m4a]]


**EDF schedulabilty condition**
--> we have a necessary and sufficient condition


![[Recording 20250513181907.m4a]]


**example of schedule** slide 41/42 --> differences between RM and EDF

- even the first task(high priority) you have jitter in EDF
- EDF has less preemptions. Has less preemptions for schedulability purposes.

RM dà maggiore predicibilità, meno jitter, ma questo li porta ad avere delle preemption.

So *why do i need RM and not always use EDF*:
- RM is simplier to implement in a OS level. Uno scheduler a priorità fissa la supportano quasi tutti. **RM is more predictable during overloads. An overload of a low priority task, doesn't affect the overload of the highest priority task**.


Con EDF, se ho overload, mi và a influenzare anche i task a più alta priorità.



![[Recording 20250513182808.m4a]]

![[Recording 20250513182825.m4a]]



