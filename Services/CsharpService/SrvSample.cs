using System.ServiceProcess;

namespace CsharpService
{
    class SrvSample : ServiceBase
    {
        public SrvSample()
        {
            // Initialization
            this.ServiceName = "SrvSample";

            // Set whether or not we handle different events
            this.CanHandlePowerEvent = true;
            this.CanHandleSessionChangeEvent = true;
            this.CanPauseAndContinue = true;
            this.CanShutdown = true;
            this.CanStop = true;
        }

        // Windows Service Entry Point
        static void Main()
        {
            ServiceBase.Run(new SrvSample());
        }

        // Dispose objects
        protected override void Dispose(bool disposing)
        {
            base.Dispose(disposing);
        }

        // Code on service start
        protected override void OnStart(string[] args)
        {
            base.OnStart(args);
        }

        // Code on service stop
        protected override void OnStop()
        {
            base.OnStop();
        }

        // Code on service pause
        protected override void OnPause()
        {
            base.OnPause();
        }

        // Code on service resume
        protected override void OnContinue()
        {
            base.OnContinue();
        }

        // Code on service response to system shutdown
        protected override void OnShutdown()
        {
            base.OnShutdown();
        }

        // Send a command (command here is Arbitrary Integer between 128 & 256)
        // directly to the service without the need for Remoting or Sockets
        protected override void OnCustomCommand(int command)
        {
            //  A custom command can be sent to a service by using this method:
            //  int command = 128; //Some Arbitrary number between 128 & 256
            //  ServiceController sc = new ServiceController("NameOfService");
            //  sc.ExecuteCommand(command);

            base.OnCustomCommand(command);
        }

        // Can be used to track power status change
        protected override bool OnPowerEvent(PowerBroadcastStatus powerStatus)
        {
            return base.OnPowerEvent(powerStatus);
        }

        // Can be used to track session change
        protected override void OnSessionChange(SessionChangeDescription changeDescription)
        {
            base.OnSessionChange(changeDescription);
        }
    }
}
