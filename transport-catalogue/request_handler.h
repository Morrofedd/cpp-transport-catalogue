#pragma once

#include "json_reader.h"
#include "transport_catalogue.h"

class request_handler
{
public:
	request_handler() = delete;
	request_handler(TransportCatalogue::TransportCatalogue&, json_reader&);

	void MakeRequest(std::ostream& output);

	void RequestAnser(std::ostream& output);

private:
	void AddInformationInCatalogue(const std::vector<std::pair<request_type, RequestValue>>& request_queue);

private:

	TransportCatalogue::TransportCatalogue& catalogue_;
	json_reader& reader_;
};

