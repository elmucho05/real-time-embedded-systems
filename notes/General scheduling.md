Scheduling is the activity of selecting which process/thread should be executed next.
We can distinguish 3 types :

Long term scheduling : before creating a process, decide if it must be activated

Medium term scheduling : decide if a process has to be swapped out/in

short term scheduling : decide which process has to be executed next



Off-line vs. On-line
static vs. dynamic

optimal vs. best-effort



**non real-time scheduling algorithms**
First Come First Served (FCFS)
Shortest Job First (SJF)
Priority Scheduling
Round Robin (RR)

all of them are not suited for real-time systems

#### FCFS
It assign the cpu to the tasks based on their arrival times.

the priority is set wit the arriving time.
It is dynamic

in linux : sched FIFO, non è uno scheduler fifo puro, è a priorità, ma FIFO riguarda task a cui è associata la stessa priorità.

Il primo arrivato a quella priorità è il primo che esegue.

it is very impredicable
![[Pasted image 20250506163442.png]]


this scheduler is very unpredictable because the finishing time of a task may significantly vary.

say more tasks arrive that wish to execute a in a determined time.


The **Response** time is : finishing time - arrival time
tau 1 = 20 - t0 ? 20
tau 2 = 28-20


**so response time really depends on the arrival time**. if i have deadlines, that scheduling algorithm doesn't help me at all.

Se controlli un motore e non controllate gli input, lui perde il passo. 

Il task che controlla l'attuatore, deve avere controllo su quando arriva quel task, non deve essere in ritardo.

Se mischio task grossi con piccolini, se gli fai andare in ordine di arrivo, è la fine.


#### Shortest Job First

- it selects the task with the shortest computation time.
- non preemptive or preemptive
	- preemptive, if a shorter task arrives, it preempts the running and give control to the shorter one
- **static** : priority is based on execution time, tutti i job dello stesso task hanno la stessa priorità
- it can be used online or off-line
- it minimizes the average response time of the tasks. 


Idea generale : cerco di schedulare prima i task che richiedono poco tempo.
Tratti prima i task meno impegnativi, così **faccio tanti, in poco tempo**. Quelle più pesanti, alla fine, almeno hai l'impressione di aver finito tanta roba.

The average response time : is the minimum possible one.

We need a theoretical proof of optimality.

**How do we provo optimality?**
Dimostrazione per assurdo, proof by contradiction.
Provo Se NON B, allora NON A.
Nego la tesi e raggiungo una conclusione nell'ipotesi.

Negare questa proprietà signifca assumere che esisteun'altro algoritmo che ha un **average response time migliore $\sigma$ che fa meglio in termine di tempo di risposta media**
![[Pasted image 20250506164951.png]]


$\sigma'$ is identical to sigma but inverting S and L.

And i'm claiming some property here. 
$fS’ < f_Lf_L’ = f_S$
The finishing time of L would be equal to finishing time of S in sigma'.

Sommando i tempo di risposta, **in sigma' faccio meglio risposta a sigma**

`Sommo tutti i tempi di risposta e divido per il numero totale di task`

`finishing time - start = response time`
sigma prima is not yet SJF, is more similar than sigma. 

$\sigma'$ fin'ora è simile, ma non so come sarà dopo.

--> semplicemente RIPETO TUTTO
![[Pasted image 20250506165408.png]]

Continuing like that, i will reach a point where sigma' is similar to sjf.

ASSUMO per assurdo che esista un algoritmo diverso da sjf che abbia un tempo minore.

fino a sigma, lui prendeva una decisione diversa.
![[Recording 20250506165627.m4a]]


Con la catena di maggiorazioni, io continuo ad applicare quella logica, ottendendo che sigma* sarà = sigma di sjf
Con delle semplice manipolazioni, provo il mio teorema. 




![[Recording 20250506165736.m4a]]


##### IS SJF suited for real-time?
it is not optimal.

Say the taasks have a particular deadline. C'è un'altro algoritmo di scheduling, che in base a quelle deadline *d*, avrebbe fatto meglio.
![[Pasted image 20250506165955.png]]

SJF è ottimo ma non per non violare le deadline, è buono per tempi di riposta medio.

Esistono altri algoritmo migliori per 
![[Recording 20250506170017.m4a]]

## Priority based schedulers
I assign a priority to tasks.
For instance in linux, the tasks with the highest priority executes, it is chosen from the ready queue.

TASKS WITH THE SAME PRIORITY may be served with FCFS or Round Robin.
Linux does this. 

Linux ha due classi :
- sched FIFO : viene sempre eseguito quello a più alta priorità, a livello di parità, usiamo FIFO
- sched RR : round robin, eseguo per dei quanti di tempo i task che appartengono alla stessa classe di priorità 



This type of scheduler, is way more suitable for motor control tasks for our project.

Can be used for real-time purposes if priorities are assigned following specific rules.

There should be a proper way to assign priorities based on deadlines.

![[Recording 20250506170604.m4a]]


**starvation** : ow priority tasks may experience long delays due to the preemption of high priority tasks. There is a problem of fairness to low priority tasks

**aging** : priority increases with waiting time. A low priority task, while waiting, increases it's priority.


if $p_i circa \frac{1}{C_i}$ assegno i task in base alla loro tempo di computazione, ottendo sjf

--> pi piccolo è il tempo di esecuzione, più alta priorità
![[Recording 20250506170907.m4a]]



## ROUND ROBIN
if the task did not reach conclusion after a time quantum, gets prempted
E' simile al fifo, ma ha un limite di tempo, raggiunte le Q unità di tempo, se il task non ha finito, viene deschedulato e reinserito in CODA alla READY QUEUE.

-> linux SCHED RR


![[Recording 20250506171103.m4a]]

Given N= number of task on the system
I have 3 tasks, after 3Q, my  blue task, will have again the possibility to execute again. So on.

The response time of my task is : *ow many big tasks do i have(triple)*
Esempio : quanti grandi cicli prima di completare la mia esecuzione, how many **big quanta** do i have to wait until i finish exec

--> **big qunata = $nQ$ $\times \frac{C_i}{Q}$**
Se ho 15 task, il mio tempo di risposta sarà 15 volte il mio execution time.
***each task runs as it was executing alone on a virtual processor n times slower than the real one.***

![[Pasted image 20250506171142.png]]

E' come andare su un processore che è n volte più lento, dove n sono gli altri task schedulati con me.
![[Recording 20250506171458.m4a]]

**Proprietà**
Se nessun task sarà mai interrotto, questo si riduce ad un FIFO.

Se faccio il quanto piccolo a piacere, è più il tempo che ci metto al cambio di contesto, che il tempo che ci metto per eseguire un task. Il mio quanto di tempo diventa Q + DELTA, cioè Q + tempo di overhead. 
DELTA = context switch time


![[Pasted image 20250506171626.png]]

Il tempo di risposta, dattando Q = Q+delta, il mio tempo di risposta da $nC_i$ a Quella roba nella formula.

Se DELTA diventa dominante, i can waste a lot of my processor due to context swithces
![[Recording 20250506171820.m4a]]


#### **IN LINUX**
![[Pasted image 20250506171848.png]]

Is there any task to execute? yes then runs,

if there is no task, i go to next priority queue, until i find a task ready to execute. 

How are the tasks in RQ1,2,n ? --> Tasks having the same priority use either RR or FIFO, depending on scheduling class.


#### multiple-feedback queues
NOT VERY USEFUL, not predictable
when a process is activated or it is unblocked, it goes in the highest priority queue RQ0

periodically, processes can be “moved up” if they were not able to execute enough (to avoid starvation)

this scheduling policy tries to overcome the penalty for I/O bound processes, it has very little amount of time to execute, and that it will wait for IO.
- a process always starts from RQ0, so it's a good idea to start it, then make it wait
- a short process, will probably complete very soon
- if the system is heavily loaded, periodically the CPU-bound processes in the last queue are “promoted” to higher priority queues



# Real-time SCHEDULERS

Algoritmi per task che hanno anche una deadline associata.
-> entro quanto devo aver completato quella task

Tasks may have relative or absolute deadlines.

### Earliest due date
considers tasks with the earliest relative deadline.
Eseguono i task con la deadline prima.

SCHEDULO PRIMA la task che ha la deadline prima degli altri.
Quello che deve finire per forza prima degli altri.
- all tasks arrive simultaneously
- assume deadline is known in advance
- preemption is not an issue
- it minimizes the maximum lateness (L_max)

**latenes** : quanto un task ha eseguito prima o dopo la sua deadline. Una latess negativa dice che un task ha completato prima della sua deadline.
--> una grande non ci piace
-> massima lateness = response time - deadline . Guardo quello che ha finito più oltre alla deadline. Minimizzare significa fare che il task che è arrivato più in ritardo di tutti, sia più vicino alla sua deadline.

MINIMIZZA LA LATENESS MASSIMA

![[Recording 20250506174601.m4a]]


***How can i prove that?***
Means i'm taking the task that is as close as possible to it's lateness
![[Pasted image 20250506174653.png]]

![[Pasted image 20250506174701.png]]


If MAX Lateness is < 0, significa che tutti i task eseguono prima della loro deadline. Avere un algoritmo che minimizza questa, significa che non ci sono task che hanno missato la deadline.
![[Recording 20250506174809.m4a]]



![[Pasted image 20250506174802.png]]

**The proof is by contradiction**:
ASSUME that sigma is different from earliest due date.
Assumo che esiste un algoritmo diverso da EDD che abbia una lateness massima migliore EDD.

Let's reach the first point in time that sigma takes a different decision from EDD
-> i get sigma'.

- taking a different decision, means that the task with an earler deadline is executed first


Let's prove that maximum lateness **Lmax = La = fa-da**

fa = fb in sigma'. Si evince dalla figura.
**in sigma'**
- Finishing time of **a** in sigma' is earlier that the one in sigma'--> **fa' < fa**
	- i scheduled A before B, so in sigma' it finishes before 
- La' = fa' - da (deadline) < fa - da which is **La = fa-fd**, so La' < Lmax
- Lb' = fb' -db = fa-db < fa -da = La = fa-da, so Lb' < Lmax 


I then repeat the same for each pair of tasks that sigma' would have taken a different choice than EDD, and then i swap A and B, and get the sigma* = sigma EDD, reaching a contraddiction

**SO THERE IS NO OTHER ALOGRITHM THAT CAN HAVE A smaller MAXIMUM lateness than EDD**

![[Pasted image 20250506175927.png]]
![[Recording 20250506175933.m4a]]


#### EDD - guarantee test
Scheduling problem : what is the best algorithm to have optimality in deadline misses.

BUT, anche avendo un algoritmo ottimo, non posso schedulare tutti i task, se c'è troppa roba si blocca.  

***PROBLEMA di SCHEDULABILITA'** : usando un algoritmo ottimo, mi devi dire se le task riusciranno a rispettare le deadline?*

Se metti troppa roba, anche se hai algoritmo ottimi, ma hai sovvracaricato il processore, non tutte le task possono "meet their deadline".



THE GUARANTEE CHECK for EDD assumes that ****
![[Pasted image 20250506180608.png]]
if all the finishing times are less then theri deadlines
- order the tasks
- if for every task, the sum is smaller than or equal to their realtive deadline, then my **feasibilty is guaranteed**. Se la somma di tutti i task, è minore del corrente, allora l'esecuzione del mio task è garantito.

![[Recording 20250506180804.m4a]]



### Earliest Deadline First - EDF  
it selects the task with the earliest absolute deadline

Tasks can arrive as they wish.
Among the ready tasks, i prioritize the ones arrived earlier. 
![[Pasted image 20250506181105.png]]
If arrives a task with earlier deadline, then i choose that.

![[Pasted image 20250506181140.png]]


EDF complexity
Quanto tempo ci vuole per inserire un nuovo task nella mia coda ordinata in base alla deadline più imminente.
- O(n) to insert a new task in the deadline ordered ready queue? Al peggio ci metto n passi per inserire un nuovo task nella coda. LAW of diminishing returns : prima o poi è più l'overhead di avere più task, che il guadagno fatto.

A noi interessa più l'estrazione che l'inserzione. E' più importante perché devo decidere quale cosa far eseguire dopo, se perdo tempo a estrarre la task imminente, perdo tempo poi anche a decidere, per questo uso una coda che come estrazione pago O(1).

*E per provare che ci stai in una deadline?*
-> è un problema NP(Hard) se Ho un multicore. diventa ESPONENZIALE dimostrarre una soluzione. 
- **single-core** abbiamo degli algoritmi per calcolarlo in tempo lineare o polinomiale.

***Quando ho bisogno di estrema predicibilità***, non faccio trasferire i task da un core all'altro ma semplicemente lo lascio sempre su un core. 


![[Recording 20250506182302.m4a]]

- O(n) to guarantee a new task





```shell
mkdri src 
colcon build
source install  ...
ros2 pkg create  --build_type ament_cmake pubsub


```


ci crea già la struttura corretta

il `package.xml` dove tra le varie cose troviamo le dipendenze


c'è anche il cmake_lists


add_executable(subnode src/sub.cpp)
add_dependencies .... INSOMMA devi creare un file sub.cpp



guarda dalle slide



L'argomento che vuole la funzione spin non è di tipo node, ma è uno sharepoint.

--> uno sharepoint è un oggetto che wrappa un puntatore e mantiene il conteggio delle referenze che puntano a quel puntatore.



