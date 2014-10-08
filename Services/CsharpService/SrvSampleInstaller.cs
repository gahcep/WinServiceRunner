using System.ComponentModel;
using System.Configuration.Install;
using System.ServiceProcess;

namespace CsharpService
{
    [RunInstaller(true)]
    public class SrvSampleInstaller : Installer
    {
        public SrvSampleInstaller()
        {
            var serviceProcessInstaller = new ServiceProcessInstaller();
            var serviceInstaller = new ServiceInstaller();

            // Service Account Information
            serviceProcessInstaller.Account = ServiceAccount.LocalSystem;
            serviceProcessInstaller.Username = null;
            serviceProcessInstaller.Password = null;

            // Service Information
            serviceInstaller.DisplayName = "Sample Service";
            serviceInstaller.StartType = ServiceStartMode.Automatic;

            // Should be the same as ServiceName in SrvWatchdog class
            serviceInstaller.ServiceName = "SrvSample";

            this.Installers.Add(serviceProcessInstaller);
            this.Installers.Add(serviceInstaller);
        }
    }
}
