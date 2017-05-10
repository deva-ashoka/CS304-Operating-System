//OS Assignment: Sleeping TA Problem
//Name: Deva

import java.util.Random;
import java.util.concurrent.Semaphore;

//Semaphore for wake up signal for the TA
class WakeUpSemaphore {
    public boolean signal = false;

    public synchronized void wakeUp() {
        this.signal = true;
        this.notify();
    }

    public synchronized void wakeUpSignalForTA() throws InterruptedException {
        while (!this.signal) wait();
        this.signal = false;
    }
}

//Student - either does programming (working) or is waiting to meet the TA or is with the TA.
class Student implements Runnable {
    int studentID;
    WakeUpSemaphore wakeUpTaSemaphore;
    Semaphore chairs;
    Semaphore TA;
    Thread t;

    public Student(WakeUpSemaphore w, Semaphore c, Semaphore a, int id) {
        wakeUpTaSemaphore = w;
        chairs = c;
        TA = a;
        studentID = id;
        t = Thread.currentThread();
    }

    public void workForSomeTime() {
        try {
            System.out.println("Student " + studentID + " began working");
            Random random = new Random();
            t.sleep(random.nextInt(5) * 1000);
            System.out.println("Student " + studentID + " finished working");
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

    }

    public void meetTA() {
        try {
            System.out.println("Student " + studentID + " began discussing with the TA");
            t.sleep(5000);
            System.out.println("Student " + studentID + " is done discussing with the TA");
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    //if the student wants to meet the TA but the TA is busy. So wait outside the office (sitting in chair) and then meet TA
    public void waitAndMeetTA() {

        try {
            System.out.println("Student " + studentID + " is waiting outside the office. "
                    + "He/she has taken chair number: " + ((3 - chairs.availablePermits())));
            TA.acquire();
            System.out.println("Student " + studentID + " began discussing with the TA.");
            t.sleep(5000);
            System.out.println("Student " + studentID + " is done discussing with the TA");
            TA.release();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

    }

    public void run() {
        while (true) {
            workForSomeTime();
            System.out.println("Student " + studentID + " wants to meet the TA");
            if (TA.tryAcquire()) {
                try {
                    wakeUpTaSemaphore.wakeUp();
                    System.out.println("Student " + studentID + " woke up the TA");
                    meetTA();
                } finally {
                    TA.release();
                }
            } else {
                System.out.println("TA is busy. Student " + studentID + " is checking for available chairs");
                if (chairs.tryAcquire()) {
                    waitAndMeetTA();
                } else {
                    System.out.println("TA is busy and the chairs are full. Student " + studentID + " went back to work");
                }
            }

        }
    }
}

//TA: is either sleeping (when no students to meet) or is with a student
class TeachingAssistant implements Runnable {

    private WakeUpSemaphore wakeUpSemaphore;
    private Semaphore chairs;
    private Thread t;

    public TeachingAssistant(WakeUpSemaphore w, Semaphore c) {
        t = Thread.currentThread();
        wakeUpSemaphore = w;
        chairs = c;
    }

    public void run() {
        while (true) {
            try {
                System.out.println("There are no students. TA is going to sleep");
                wakeUpSemaphore.wakeUpSignalForTA();
                t.sleep(1000);
                if (chairs.availablePermits() != 3) {
                    do {
                        t.sleep(5000);
                        chairs.release();
                    }
                    while (chairs.availablePermits() != 3);
                }
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }
}

public class SleepingTA {

    public static int numberOfStudents;
    public static int numberOfChairs;

    public static void main(String[] args) {

        numberOfChairs = Integer.parseInt(args[0]);
        numberOfStudents = Integer.parseInt(args[1]);

        WakeUpSemaphore wakeUpTASemaphore = new WakeUpSemaphore();
        Semaphore chairs = new Semaphore(numberOfChairs);
        Semaphore TA = new Semaphore(1);

        Thread threadTA = new Thread(new TeachingAssistant(wakeUpTASemaphore, chairs));
        System.out.println("------Office hours of TA started------");
        threadTA.start();

        for (int i = 0; i < numberOfStudents; i++) {
            Thread threadStudent = new Thread(new Student(wakeUpTASemaphore, chairs, TA, i + 1));
            threadStudent.start();
        }


    }
}





