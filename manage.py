import os, argparse, shutil, datetime, time, glob, json

# createFolder
def createFolder( path ):
    if os.path.exists( path ):
        return

    print 'Creating {0} folder...'.format( path )
    os.makedirs( path )

# install
def install( info ):
    print 'Installing...'

    os.system( 'export LC_ALL=C && mongorestore --drop -h localhost:27017' )

    if os.path.exists( 'dump' ):
	    shutil.rmtree( 'dump' )

    os.system( 'sudo cp {0} /etc/init/{1}'.format( info['service']['conf'], os.path.basename( info['service']['conf'] ) ) )

    os.system( 'npm install -d --unsafe-perm' )

# start
def start( info ):
    print 'Starting...'

    os.system( 'sudo stop {0}; sudo start {0}'.format( info['service']['name'] ) )

# backup
def backup( info ):
    print 'Backup...'

    createFolder( 'backup' )

    if info['backup']:
	    dumpDatabases( info['backup']['db'], 'backup' )

    os.system( 'zip backup-{0}.zip backup -0 -r'.format( datetime.datetime.now().strftime('%s') ) )

    shutil.rmtree( 'backup' )

# restore
def restore( info ):
    print 'Restore...'

    # Find the latest backup
    newest = max( glob.iglob('backup-*.zip'), key=os.path.getctime )

    os.system( 'unzip {0}'.format( newest.replace( '.zip', '' ) ) )

#    if os.path.exists( 'app/uploads' ):
#        shutil.rmtree( 'app/uploads' )
#
#    shutil.copytree( 'backup/uploads', 'app/uploads' )

    os.system( 'export LC_ALL=C && cd backup && ls && mongorestore --drop -h localhost:27017')

    shutil.rmtree( 'backup' )

# upload
def upload( info ):
	print 'Uploading to {0}:{1}...'.format( info['host'], info['path'] )
	os.system( "rsync --exclude-from=ignore.txt -avz . {0}:{1}".format( info['host'], info['path'] ) )
	os.system( "ssh -t {0} 'cd {1} && python manage.py start'".format( info['host'], info['path'] ) )

# deploy
def deploy( info ):
    print 'Deploying to {0}:{1}...'.format( info['host'], info['path'] )

    if info['deploy']:
	    dumpDatabases( info['deploy']['db'] )

    os.system( "rsync --exclude-from=ignore.txt -avz . {0}:{1}".format( info['host'], info['path'] ) )
    os.system( "ssh -t {0} 'cd {1} && python manage.py install start'".format( info['host'], info['path'] ) )

# dumpDatabases
def dumpDatabases( items, folder = None ):
	for item in items:
		db = item.split( ':' )
		if folder:
			os.system( 'export LC_ALL=C && cd {0} && mongodump -d {1} -c {2}'.format( folder, db[0], db[1] ) )
		else:
			os.system( 'export LC_ALL=C && mongodump -d {0} -c {1}'.format( db[0], db[1] ) )

# main
def main():
    parser = argparse.ArgumentParser(description='LolHub deployment tool')
    parser.add_argument( 'actions', type=str, nargs='+', help='deployment action' )

    args = parser.parse_args()

    # Available actions
    Actions = { 'install': install, 'deploy': deploy, 'start': start, 'backup': backup, 'restore': restore, 'upload': upload }

    # Parse deployment info
    with open( 'deploy.json' ) as fh:
        info = json.loads( fh.read() )

    if info == None:
        print 'No deploy.json found'
        return

    # Run actions
    for action in args.actions:
        if not action in Actions.keys():
            print 'Unknown action ' + action
            continue

        Actions[action]( info )

# Run main
if __name__ == '__main__':
    main()