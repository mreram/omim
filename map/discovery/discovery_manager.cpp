#include "map/discovery/discovery_manager.hpp"

namespace
{
std::string GetQuery(discovery::ItemType const type)
{
  switch (type)
  {
  case discovery::ItemType::Hotels: return "hotel ";
  case discovery::ItemType::Attractions: return "attraction ";
  case discovery::ItemType::Cafes: return "cafe ";
  case discovery::ItemType::LocalExperts:
  case discovery::ItemType::Viator: ASSERT(false, ()); return "";
  }
}
}  // namespace

namespace discovery
{
Manager::Manager(Index const & index, search::CityFinder & cityFinder, APIs const & apis)
  : m_index(index)
  , m_cityFinder(cityFinder)
  , m_searchApi(apis.m_search)
  , m_viatorApi(apis.m_viator)
  , m_localsApi(apis.m_locals)
{
}

// static
search::SearchParams Manager::GetSearchParams(Manager::Params const & params, ItemType const type)
{
  search::SearchParams p;
  p.m_query = GetQuery(type);
  p.m_inputLocale = params.m_lang;
  p.m_viewport = params.m_viewport;
  p.m_position = params.m_viewportCenter;
  p.m_maxNumResults = params.m_itemsCount;
  p.m_mode = search::Mode::Viewport;
  return p;
}

std::string Manager::GetCityViatorId(m2::PointD const & point) const
{
  ASSERT_THREAD_CHECKER(m_threadChecker, ());
  auto const fid = m_cityFinder.GetCityFeatureID(point);
  if (!fid.IsValid())
    return {};

  Index::FeaturesLoaderGuard const guard(m_index, fid.m_mwmId);
  FeatureType ft;
  if (!guard.GetFeatureByIndex(fid.m_index, ft))
  {
    LOG(LERROR, ("Feature can't be loaded:", fid));
    return {};
  }

  auto const & metadata = ft.GetMetadata();
  auto const sponsoredId = metadata.Get(feature::Metadata::FMD_SPONSORED_ID);
  return sponsoredId;
}
}  // namespace discovery