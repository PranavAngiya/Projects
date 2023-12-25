import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

public class AuctionScheduler {

    private static final long INITIAL_DELAY = 0;  // Initial delay before starting execution
    private static final long PERIOD = 1;  // Run every 1 minute

    public static void main(String[] args) {
        ScheduledExecutorService scheduler = Executors.newScheduledThreadPool(1);

        scheduler.scheduleAtFixedRate(AuctionProcessor::processAuctions, INITIAL_DELAY, PERIOD, TimeUnit.MINUTES);
    }
}