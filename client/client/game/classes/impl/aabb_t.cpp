#include "pch.hpp"
#include "game_classes.hpp"

aabb<double> aabb_t::bounds() const
{
	if (!instance_object_) {
		return {};
	}

	static java_field_t min_x = get_mapped_field(aabb_class_, "minX");
	static java_field_t min_y = get_mapped_field(aabb_class_, "minY");
	static java_field_t min_z = get_mapped_field(aabb_class_, "minZ");
	static java_field_t max_x = get_mapped_field(aabb_class_, "maxX");
	static java_field_t max_y = get_mapped_field(aabb_class_, "maxY");
	static java_field_t max_z = get_mapped_field(aabb_class_, "maxZ");

	if (!min_x || !min_y || !min_z || !max_x || !max_y || !max_z) {
		return {};
	}

	return
	{
		min_x.get<jdouble>(instance_object_),
		min_y.get<jdouble>(instance_object_),
		min_z.get<jdouble>(instance_object_),
		max_x.get<jdouble>(instance_object_),
		max_y.get<jdouble>(instance_object_),
		max_z.get<jdouble>(instance_object_),
	};
}

void aabb_t::set_bounds(const aabb<double>& bounds) const
{
	if (!instance_object_) {
		return;
	}

	static java_field_t min_x = get_mapped_field(aabb_class_, "minX");
	static java_field_t min_y = get_mapped_field(aabb_class_, "minY");
	static java_field_t min_z = get_mapped_field(aabb_class_, "minZ");
	static java_field_t max_x = get_mapped_field(aabb_class_, "maxX");
	static java_field_t max_y = get_mapped_field(aabb_class_, "maxY");
	static java_field_t max_z = get_mapped_field(aabb_class_, "maxZ");

	if (!min_x || !min_y || !min_z || !max_x || !max_y || !max_z) {
		return;
	}

	min_x.set<jdouble>(bounds.min_x);
	min_y.set<jdouble>(bounds.min_y);
	min_z.set<jdouble>(bounds.min_z);
	max_x.set<jdouble>(bounds.max_x);
	max_y.set<jdouble>(bounds.max_y);
	max_z.set<jdouble>(bounds.max_z);
}
