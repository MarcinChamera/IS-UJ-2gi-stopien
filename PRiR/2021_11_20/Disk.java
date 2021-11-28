import java.util.concurrent.atomic.AtomicIntegerArray;
import java.util.Random;

class Disk implements DiskInterface {
    private AtomicIntegerArray array;
    private int size = 5;
    private Random random;

    Disk() {
        array = new AtomicIntegerArray(size);
        random = new Random();
        for (int i = 0; i < size; i++) {
            array.set(i, random.nextInt(size));
        }
    }

    public void write(int sector, int value) throws DiskError {
        Random random = new Random();
        int throwDiskError = random.nextInt(4);
        if (throwDiskError != 0) {
            array.set(sector, value);
        }
        else {
            throw new DiskError();
        }
    }

    public int read(int sector) throws DiskError {
        // Random random = new Random();
        // int throwDiskError = random.nextInt(4);
        // if (throwDiskError == 0) {
        //     System.out.println("DiskError dla sektora " + sector + ", proba odczytu");
        //     throw new DiskError();
        // }
        return array.get(sector); 
    }

    public int size() {
        return size;
    }

    public String showValuesInDisk() {
        // System.out.println(array.toString());
        return array.toString();
    }
}