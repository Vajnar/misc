/*
 * Efficient Tabu search algorithm implementation
 * for solving Pj, Dj, Wj problems with respect
 * to minimizing total weighted tardiness
 *
 * by Martin Vajnar, 2013-5
 *
 */

public class Tabu {

	static final int TABU_LENGTH = 11;
	static final int PASSES  = 200;
	static final int PASS_WIDTH = (int)Math.log10(PASSES)+1;
	static final int ID_WIDTH, PJ_WIDTH, DJ_WIDTH, WJ_WIDTH;

	static final Task[] tasks_initial = {	new Task (16,  67, 45,  2),
						new Task ( 6, 105, 35,  3),
						new Task (12,   8, 80, 15),
						new Task (19, 124, 28,  6),
						new Task ( 9,  77,  1,  5),
						new Task (20, 202, 70, 10),
						new Task (13, 157, 14,  8),
						new Task ( 1, 194, 21,  7),
						new Task ( 5,   5, 69, 13),
						new Task (18,   7, 62, 14),
						new Task ( 4,  36, 21,  1),
						new Task ( 5,  53, 73,  4),
						new Task (19,  61, 23, 12),
						new Task (12,  25, 76,  9),
						new Task (20,  43, 51, 11) };
	static Task[][] tabu = new Task[TABU_LENGTH][2];
	static int entry_id = 0;

	static {
		int id=1, pj=1, dj=1, wj=1;

		if (TABU_LENGTH >= (tasks_initial.length * (tasks_initial.length - 1)) / 2) {
			System.err.println("Tabu list is too long.");
			System.exit(1);
		}

		for(Task task : tasks_initial) {
			if (task.id > id)
				id = task.id;
			if (task.pj > pj)
				pj = task.pj;
			if (task.dj > dj)
				dj = task.dj;
			if (task.wj > wj)
				wj = task.wj;
		}

		ID_WIDTH = (int)Math.log10(id)+1;
		PJ_WIDTH = (int)Math.log10(pj)+1;
		DJ_WIDTH = (int)Math.log10(dj)+1;
		WJ_WIDTH = (int)Math.log10(wj)+1;
	}

	static void iter_printout(Task[] tasks, int fitness, int iter) {
		String buffer = String.format("%" + PASS_WIDTH + "d: ", iter);

		for(Task task : tasks)
			buffer += String.format("%" + ID_WIDTH + "d, ",task.id);

		buffer += String.format("(%d)",fitness);
		System.out.println(buffer);
	}

	static void printout(Task[] tasks, int fitness) {
		String buffer = "Initial schedule (Id: Pj, Dj, Wj):\n";

		for (Task task : tasks)
			buffer += task;

		buffer += String.format("Fitness: %d\n\nIteration step: best schedule, (fitness):",fitness);

		System.out.println(buffer);
		iter_printout(tasks,fitness,0);
	}

	static void swap(Task[] a, int i) {
		Task temp = a[i];

		a[i] = a[i+1];
		a[i+1] = temp;
	}

	static boolean is_in_tabu(Task a, Task b) {
		for (Task[] entry : tabu)
			if (entry[0] == a && entry[1] == b)
				return true;
		return false;
	}

	static void add_to_tabu(Task a, Task b) {
		tabu[entry_id][0] = a;
		tabu[entry_id][1] = b;

		if (entry_id == tabu.length-1) entry_id = 0;
		else entry_id++;
	}

	static int compute_initial_fitness() {
		int time = 0, fitness = 0;

		for (Task task : tasks_initial) {
			int tj;

			time += task.pj;
			if ((tj = time - task.dj) > 0)
				fitness += tj * task.wj;
		}

		return fitness;
	}

	static int compute_fitness(Task a, Task b, int fitness_prev_it, int time) {
		int tj;

		time += a.pj;
		if((tj = time - a.dj) > 0)
			fitness_prev_it -= tj * a.wj;

		time += b.pj;
		if((tj = time - b.dj) > 0)
			fitness_prev_it -= tj * b.wj;

		time -= a.pj;
		if((tj = time - b.dj) > 0)
			fitness_prev_it += tj * b.wj;

		time += a.pj;
		if((tj = time - a.dj) > 0)
			fitness_prev_it += tj * a.wj;

		return fitness_prev_it;
	}

	public static void main(String[] args) {
		int fitness_best, fitness_prev_it, pass = 0;
		Task[] tasks_best = tasks_initial, tasks_prev_it = tasks_initial;

		fitness_best = fitness_prev_it = compute_initial_fitness();
		printout(tasks_best,fitness_best);

		for (int i = 1; i <= PASSES; i++) {
			int perm = -1, fitness_curr_it = Integer.MAX_VALUE;

			for (int j = 0, time = 0; j < tasks_prev_it.length-1; time += tasks_prev_it[j++].pj) {
				if (!is_in_tabu(tasks_prev_it[j],tasks_prev_it[j+1])) {
					int fitness_temp = compute_fitness(tasks_prev_it[j],tasks_prev_it[j+1],fitness_prev_it,time);
					if (fitness_temp < fitness_curr_it) {
						fitness_curr_it = fitness_temp;
						perm = j;
					}
				}
			}

			swap(tasks_prev_it,perm);
			add_to_tabu(tasks_prev_it[perm],tasks_prev_it[perm+1]);

			fitness_prev_it = fitness_curr_it;
			if (fitness_curr_it < fitness_best) {
				fitness_best = fitness_curr_it;
				tasks_best = tasks_prev_it.clone();
				pass = i;
			}

			iter_printout(tasks_prev_it,fitness_curr_it,i);
		}

		System.out.println("\nBest schedule:");
		iter_printout(tasks_best,fitness_best,pass);
	}
}

class Task {

	int pj, dj, wj, id;

	Task(int pj, int dj, int wj, int id) {
		this.pj = pj;
		this.dj = dj;
		this.wj = wj;
		this.id = id;
	}

	@Override
	public String toString() {
		return String.format("%" + Tabu.ID_WIDTH + "d: %" + Tabu.PJ_WIDTH + "d, %" + Tabu.DJ_WIDTH + "d, %" + Tabu.WJ_WIDTH + "d\n",
			this.id,this.pj,this.dj,this.wj);
	}
}
