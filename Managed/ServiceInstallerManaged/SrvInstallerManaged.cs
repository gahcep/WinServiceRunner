using System;
using System.ComponentModel;
using System.Runtime.InteropServices;

namespace WinSrvRunnerManaged
{
    // According to 
    // http://msdn.microsoft.com/en-us/library/windows/desktop/ms685996(v=vs.85).aspx
    public enum WinSrvState
    {
        ServiceStopped          = 1,
        ServiceStartPending     = 2,
        ServiceStopPending      = 3,
        ServiceRunning          = 4,
        ServiceContinuePending  = 5,
        ServicePausePending     = 6,
        ServicePaused           = 7
    }

    public static class SrvInstallerManaged
    {
        #region Error codes

        public const Int32 InstallerOpSuccess                   = 0;
        public const Int32 InstallerErrorOpenScManager          = 0x100;
        public const Int32 InstallerErrorCreateService          = 0x101;
        public const Int32 InstallerErrorOpenService            = 0x102;
        public const Int32 InstallerErrorQueryService           = 0x103;
        public const Int32 InstallerErrorStartService           = 0x104;
        public const Int32 InstallerErrorStopService            = 0x105;
        public const Int32 InstallerErrorPauseService           = 0x106;
        public const Int32 InstallerErrorContinueService        = 0x107;
        public const Int32 InstallerErrorInvalidState           = 0x108;
        public const Int32 InstallerErrorTransitionFailure      = 0x109;
        public const Int32 InstallerErrorWrongAction            = 0x110;
        public const Int32 InstallerErrorInvalidHandle          = 0x111;
        public const Int32 InstallerErrorAccessDenied           = 0x112;
        public const Int32 InstallerErrorDeleteService          = 0x113;
        public const Int32 InstallerErrorInvalidName            = 0x114;
        public const Int32 InstallerErrorServiceDoesNotExist    = 0x115;

        #endregion

        #region Native functions

        [DllImport("WinSrvRunner.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I4)]
        public static extern Int32 ServiceQueryStateByName([In] String srvName, [Out] out Int32 scState);

        [DllImport("WinSrvRunner.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool ServiceExists([In] String srvName);

        [DllImport("WinSrvRunner.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I4)]
        public static extern Int32 ServiceInstall([In] String srvName, [In] String srvFullName, [In] String srvPath);

        [DllImport("WinSrvRunner.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I4)]
        public static extern Int32 ServiceUninstall([In] String srvName);

        [DllImport("WinSrvRunner.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I4)]
        public static extern Int32 ServiceStart([In] String srvName);

        [DllImport("WinSrvRunner.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I4)]
        public static extern Int32 ServiceStop([In] String srvName);

        [DllImport("WinSrvRunner.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I4)]
        public static extern Int32 ServicePause([In] String srvName);

        [DllImport("WinSrvRunner.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I4)]
        public static extern Int32 ServiceResume([In] String srvName);

        #endregion

        #region Wrapper functions

        public static WinSrvState GetServiceStateByName(String srvName)
        {
            if (String.IsNullOrEmpty(srvName))
                throw new ArgumentNullException("srvName");

            int scState;
            int win32Error = ServiceQueryStateByName(srvName, out scState);

            if (win32Error != 0)
                throw new Win32Exception(win32Error, "WinSrvRunner error");

            return (WinSrvState) Enum.ToObject(typeof (WinSrvState), scState);
        }

        #endregion

    }
}
