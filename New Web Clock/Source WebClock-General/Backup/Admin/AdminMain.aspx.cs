﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;

namespace ASPWebclock.Admin
{
    public partial class AdminMain : System.Web.UI.Page
    {
        protected void Page_Load(object sender, EventArgs e)
        {
            if(User.Identity.Name!="admin")
            {
                Response.Redirect("~/MainForm.aspx");
            }
            
        }
    }
}