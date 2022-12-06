#include "peer/Leecher.hpp"
#include "peer/Seeder.hpp"
#include "AppConductor.hpp"

namespace BT {
    AppConductor::AppConductor(StartParams const& p)
                : mParams(p) {}

    void AppConductor::Start() const {
        if (mParams.helpRequested) {
            std::cout << StartParams::GetHelpMesssage() << std::endl;
            return;
        }

        STATUSCODE status = STATUSCODE::SC_SUCCESS;
        Torrent_t torrent(mParams.torrentFilename, status);
        if (SC_FAILED(status)) {
            return;
        }

        if (mParams.enableVerbose) {
            std::cout << mParams << std::endl;
            std::cout << torrent << std::endl;
        }

        if(mParams.IsSeeder()) {
            startSeeder(torrent);
        } else {
            startLeechers(torrent);
        }
    }

    void AppConductor::startSeeder(Torrent_t const& torrent) const 
    {
        Seeder_t s(torrent, mParams.seederPort);
        s.StartTransfer();
    }

    void AppConductor::startLeechers(Torrent_t const& torrent) const 
    {
        for (auto& seeder : mParams.peers) 
        {
            Leecher_t l(torrent, seeder);
            l.startTransfer();
        }
    }
}

		
#if 0
	void *threadData = NULL;

	pthread_t seederId[cn_MaxConnections];
	bool      isTransfered = false;
	
	/* Starting thread for each leecher(Client) */
	for(i=0; i < cn_MaxConnections && peers[i] != NULL; i++)
		seederId[i] = 0;		    
	alivePeers = i;
		
	for(i=0; i < cn_MaxConnections; i++)
	{
		seederId[i] = 0;
		if(peers[i] != NULL)
		{
			_M_N_TMPARR[i] = i;
			threadData = (void *) (&_M_N_TMPARR[i]); /* indicating Nth peer */
			pthread_create(&seederId[i], NULL,  startClient, threadData);
		}
	}
		
	/* Waiting for the transfer to complete */
	while(! isTransfered && alivePeers > 0)
	{
		isTransfered = true;
		for(i=0; i< t.getNumPieces(); i++)
			if(piecesInfo[i].state != PC_COMPLETE)
				isTransfered = false;
	}

	/* Transfer complete     */
	/* Assembling the pieces */
	if(isTransfered)
	{
        ifstream  piece;
	    ofstream  save;
	    char pieceName[cn_MaxBuf] = "";
	    
		cout << "Completing the transfer.." << endl;
		log("COMPLETING THE TRANSFER");
		save.open(args.getSaveFile().c_str(), ios::binary);

		if(save.is_open())
		{
			char _tmp[1+1];
	
			for(i=0; i< t.getNumPieces(); i++)
			{
				/* Generating the piece name */
				memset(pieceName, '\0', cn_MaxBuf);
				sprintf(pieceName,".%s_%d", args.getSaveFile().c_str(), (i+1));
		
				piece.open(pieceName, ios::binary);
		
				if(piece.is_open())
				{
					while(1)
					{
						char ch = (char) piece.get();
				
						if(piece.eof())
						{
							piece.close();
							break;
						}
				
						_tmp[0] = ch;
						_tmp[1] = '\0';
				
						save.write(_tmp, 1) ;
						save.flush();
					}
			
					remove(pieceName);
				}

			}
	
			log("TRANSFER COMPLETE");
			cout << args.getSaveFile() << " completed." << endl;
			save.close();
		}
	}
	
	pthread_exit(NULL);
#endif
