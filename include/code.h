#ifndef CODE_H
#define CODE_H

#include <QTemporaryDir>

#include <string>

#include <submit-code.pb.h>

class Code
{
public:
	explicit Code(const dicoprotos::SubmitCode &sc);
	
	std::string id() const { return _id; }
	
private:
	std::string _id;
	QTemporaryDir _dir;
};

#endif
