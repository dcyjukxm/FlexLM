Instructions for Sending Out FLEXlm Developer News

Before starting this procedure, give Loree about 1/2 day notice that this is going out.

1) Prepare the Developer News html file:

	flexnews_xx.htm

   where "xx" is the next number in sequence.

2) Check the dev news into CVS at the following location:
	<cvs repository>/www.globes/public_html/FLEX/flexnews 
   which translates to
    	http://www.globes.com/FLEX/flexnews 

3) Generate list of email addresses:

	rlogin to birchlake (for its Java setup)
	cd ~pattyh/Workarea/scripts/products
	sh flexnews.sh yyyy-mm-dd

   The following is output from the script:
  
	step 1 -- contracts from express_maint_contracts which expire >= 2002-11-01
	step 2 -- maint-num from order_items where contracts == step1 output
	step 3 -- company-id from supportContacts where main-num == step2 output
	step 3a -- person-id from supportContacts where (same as step3)
	step 4 -- email from people where category flex and company == step3 output
	step 4a -- email from people where person_id == step 3a output
	final output in flexnews.2002.10.29
    
    The output file, flexnews.2002.10.29 in this example, should have 1000's 
    of email addresses.


4) Prepare the list of contacts:

	cp flexnews.2002.10.29 ~pattyh/Flexnews/maillist
	cd ~pattyh/Flexnews (it has to be this specific directory)
	chmod 644 maillist


5) Email the list out:

	a) Set up an account for 'infomail' in Outlook Express.
	    - The account has these specifics:
	      email address = infomail@macrovision.com
	      passwd = gsi4infomail
	      name = Macrovision Info Services
	    - Set up an address book entry for gsi_list@globes.com. **	 
	    - Set up and address book entry named FLEXlm Developers
	      whose email address  is infomail@macrovision.com
        
	b) Open up flexnews_xx.htm in FrontPage and select File|Send.
	    An Outlook Express window opens up with the dev notes displayed in the 
	    body of the email. (You need to configure FrontPage so that Outlook
	    Express is its default mailer: Options -> Proxy Settings, select Programs tag,
                    set e-mail to "Outlook Express".)

	c) In the Outlook Express Window, address the email as follows
	         to: FLEXlm Developers
	         bcc: gsi_list@globes.com
	         subject: FLEXlm Developers News - November 2002

	d) As the first line of the message, preceeding the html text, add: 
	          If you cannot view the message below, click here. To access 
	      the "click here" link, you need a FLEXlm developer user name 
	      and passwd. 
	    Where the "click here" link points to:
	          www.globes.com/FLEX/flexnews/flexnews_xx.htm

  **  "gsi_list" is an email alias which grabs the contents of
      "maillist."  This alias is owned and maintained by sysadmin and is
      defined as:
          :include:/u2/pattyh/Flexnews/maillist


6) Let Loree know that this has been sent
   out.  She could receive several bounces into the infomail
   box for obsolete email addresses.


7) Afer an hour or so, the delivery should be complete. To confirm, 
   check your email inbox for your personal copy. (all_gsi@globes.com
   is included in "maillist")


8) Update the file:
	<cvs repository>/www.globes/public_html/FLEX/flexnews/index.htm


	


