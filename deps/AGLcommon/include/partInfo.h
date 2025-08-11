#pragma once

// STL
#include <filesystem>
#include <string>
#include <stdint.h>
#include <vector>
// AGLcommon
#include <idCommon.h>

namespace autis
{

	struct PartIdTag {};
	using PartId = UnsignedIntIdentifier<PartIdTag>;
	const PartId INVALID_PART_ID = PartId(INVALID_UNSIGNED_INT_ID_VALUE);
	const PartId FIRST_PART_ID = PartId(1);

	struct RSVSPartIdTag {};
	using RSVSPartId = UnsignedIntIdentifier<RSVSPartIdTag>;
	const RSVSPartId INVALID_RSVS_PART_ID = RSVSPartId(0);
	const RSVSPartId FIRST_RSVS_PART_ID = RSVSPartId(1);

	struct PartInfo
	{
		std::string           container; /* name of the container to which this part belongs */
		std::string           name;      /* name of the part */
		std::filesystem::path path;      /* filename of the part */
		PartId                id;        /* part id */

		PartInfo() : container(""), name(""), path(""), id(0) {};
		PartInfo(const std::string& container, const std::string& name, const std::filesystem::path& path, PartId id)
			: container(container), name(name), path(path), id(id) {}
	};

	struct VehiclePartInfo : public PartInfo
	{
		std::vector<std::string> submodels; /* submodels to which this part belongs */
		std::vector<std::string> variants;  /* variants to which this part belongs */
		bool                     contour;   /* is a contour piece */
		RSVSPartId               rSVS_id;   /* rSVS part id (auxiliar Id needed in rSVS tunnels' conversion) */
		bool                     isDefault; /* belongs to the default submodel */
		bool                     inner;     /* inner piece of the vehicle body (an inner layer of the car) */
		bool                     real;      /* represents a real inspection piece (for example synthetic CAD parts are NOT real)*/

		VehiclePartInfo() : PartInfo(), contour(false), rSVS_id(0), isDefault(true), inner(false), real(true) {}
		VehiclePartInfo(const std::string& container, const std::string& name, const std::filesystem::path& path,
			PartId id, const std::vector<std::string>& submodels, const std::vector<std::string>& variants,
			bool contour = false, RSVSPartId rSVS_id = INVALID_RSVS_PART_ID, bool isDefault = true, bool inner = false,
			bool real = true)
			: PartInfo(container, name, path, id), submodels(submodels), variants(variants), contour(contour),
			rSVS_id(rSVS_id), isDefault(isDefault), inner(inner), real(real) {}
	};

}
