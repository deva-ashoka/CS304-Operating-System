import random
import threading
import time


class Philosopher(threading.Thread):
    def __init__(self, id, leftFork, rightFork):
        threading.Thread.__init__(self)
        self.id = id
        self.leftFork = leftFork
        self.rightFork = rightFork

    def run(self):
        while True:
            print("Philosopher" + str(self.id) + " is thinking.")
            time.sleep(1)
            print("Philosopher" + str(self.id) + " stopped thinking and wants to eat.")
            self.pickUpLeftFork()
            time.sleep(1)
            self.pickUpRightFork()
            print("Philosopher" + str(self.id) + " starts eating")
            time.sleep(random.uniform(1, 10))
            print("Philosopher" + str(self.id) + " is done eating.")
            self.putDownForks()

    def pickUpLeftFork(self):
        self.leftFork.acquire(True)

    def pickUpRightFork(self):
        self.rightFork.acquire(True)

    def putDownForks(self):
        self.leftFork.release()
        self.rightFork.release()


numberOfPhilosophers = 5
forks = [threading.Lock() for n in range(numberOfPhilosophers)]
philosopherID = [1, 2, 3, 4, 5]

philosophers = [Philosopher(philosopherID[i], forks[i % 5], forks[(i + 1) % 5]) for i in range(5)]

for x in philosophers:
    x.start()
