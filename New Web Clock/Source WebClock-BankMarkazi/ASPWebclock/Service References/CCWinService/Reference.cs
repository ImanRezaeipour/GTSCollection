﻿//------------------------------------------------------------------------------
// <auto-generated>
//     This code was generated by a tool.
//     Runtime Version:4.0.30319.17929
//
//     Changes to this file may cause incorrect behavior and will be lost if
//     the code is regenerated.
// </auto-generated>
//------------------------------------------------------------------------------

namespace ASPWebclock.CCWinService {
    
    
    [System.CodeDom.Compiler.GeneratedCodeAttribute("System.ServiceModel", "4.0.0.0")]
    [System.ServiceModel.ServiceContractAttribute(ConfigurationName="CCWinService.ICCService")]
    public interface ICCService {
        
        [System.ServiceModel.OperationContractAttribute(Action="http://tempuri.org/ICCService/CalculateOnePerson", ReplyAction="http://tempuri.org/ICCService/CalculateOnePersonResponse")]
        bool CalculateOnePerson(string barcode, string year, string month, string encryptedCode);
        
        [System.ServiceModel.OperationContractAttribute(Action="http://tempuri.org/ICCService/CalculateListPerson", ReplyAction="http://tempuri.org/ICCService/CalculateListPersonResponse")]
        bool CalculateListPerson(string[] barcodeList, string year, string month, string encryptedCode);
    }
    
    [System.CodeDom.Compiler.GeneratedCodeAttribute("System.ServiceModel", "4.0.0.0")]
    public interface ICCServiceChannel : ASPWebclock.CCWinService.ICCService, System.ServiceModel.IClientChannel {
    }
    
    [System.Diagnostics.DebuggerStepThroughAttribute()]
    [System.CodeDom.Compiler.GeneratedCodeAttribute("System.ServiceModel", "4.0.0.0")]
    public partial class CCServiceClient : System.ServiceModel.ClientBase<ASPWebclock.CCWinService.ICCService>, ASPWebclock.CCWinService.ICCService {
        
        public CCServiceClient() {
        }
        
        public CCServiceClient(string endpointConfigurationName) : 
                base(endpointConfigurationName) {
        }
        
        public CCServiceClient(string endpointConfigurationName, string remoteAddress) : 
                base(endpointConfigurationName, remoteAddress) {
        }
        
        public CCServiceClient(string endpointConfigurationName, System.ServiceModel.EndpointAddress remoteAddress) : 
                base(endpointConfigurationName, remoteAddress) {
        }
        
        public CCServiceClient(System.ServiceModel.Channels.Binding binding, System.ServiceModel.EndpointAddress remoteAddress) : 
                base(binding, remoteAddress) {
        }
        
        public bool CalculateOnePerson(string barcode, string year, string month, string encryptedCode) {
            return base.Channel.CalculateOnePerson(barcode, year, month, encryptedCode);
        }
        
        public bool CalculateListPerson(string[] barcodeList, string year, string month, string encryptedCode) {
            return base.Channel.CalculateListPerson(barcodeList, year, month, encryptedCode);
        }
    }
}
