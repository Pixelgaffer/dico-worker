#include "code.h"

using namespace dicoprotos;

#include <QDir>
#include <QFileInfo>

#include <stdlib.h>

#include <iostream>
using namespace std;

#include <archive.h>
#include <archive_entry.h>

Code::Code(const SubmitCode &sc)
	: _dir(QDir::temp().absoluteFilePath("dico-worker-XXXXXX"))
{
	if (!_dir.isValid())
		cerr << "error with tempdir" << endl;
	cout << _dir.path().toStdString() << endl;
	
	archive *a = archive_read_new();
	archive_read_support_filter_all(a);
	archive_read_support_format_all(a);
	string data = sc.archive();
	char datac[data.size()];
	data.copy(datac, data.size());
	int ret = archive_read_open_memory(a, datac, data.size());
	if (ret != ARCHIVE_OK)
	{
		cerr << "ERROR in " __FILE__ ":" << __LINE__ << endl;
		archive_read_free(a);
		return;
	}
	
	archive_entry *entry;
	while (archive_read_next_header(a, &entry) == ARCHIVE_OK)
	{
		cout << archive_entry_pathname(entry) << endl;
		switch (archive_entry_filetype(entry))
		{
		case AE_IFDIR: {
				QDir dir(_dir.path() + "/" + archive_entry_pathname(entry));
				QString name = dir.dirName();
				dir.cdUp();
				dir.mkdir(name);
			}
			break;
		case AE_IFREG: {
				QFile file(_dir.path() + "/" + archive_entry_pathname(entry));
				file.open(QIODevice::WriteOnly);
				size_t size;
				char buf[8192];
				while ((size = archive_read_data(a, buf, 8192)) > 0)
					file.write(buf, size);
				file.close();
			}
			break;
		default:
			cerr << "unrecognized type of archive entry: " << archive_entry_pathname(entry) << " (" << archive_entry_filetype(entry) << ")" << endl;
		}
	}
}
