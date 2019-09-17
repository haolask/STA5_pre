    /*
     * SHARED_DATA_SIZE bytes are reserved for M3/AP shared data
     */
    .shared_data (SHARED_DATA_BASE) (NOLOAD) :
    {
	*(.shared_data)
    }
    . = ASSERT(SIZEOF(.shared_data) <= SHARED_DATA_SIZE , "error: shared_data is too large to fit in memory segment");

    .mailboxes (MAILBOXES_BASE) (NOLOAD) :
    {
	*(.mailboxes)
    }
	. = ASSERT(SIZEOF(.mailboxes) <= MAILBOXES_SIZE , "error: mailboxes is too large to fit in memory segment");
