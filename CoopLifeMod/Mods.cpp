#include "Mods.h"
#include "Offsets.h"

Mods::Mods(HWND hWnd, int width, int height) : m_hWnd(hWnd), m_hook(new D3DHook(width, height))
{}

void Mods::Init()
{
	m_hook->setSolo(false);
	
	Player swag;
	Stats stat;

	std::vector<Player> moreSwag;
	moreSwag.push_back(swag);
	Data swagOverflow = { moreSwag };
	m_hook->setStats(swagOverflow);

	m_hook->initD3D(m_hWnd);
	m_net = new Net(m_hook);

	m_mem.Open("Risk of Rain");
}

void Mods::Loop()
{
	if (m_isConnect)
	{	
		int pause = m_mem.GetDouble(pause_offsets);
		
		ShowHUD();

		Player p;
		p.stats = m_stats;
		p.pseudo = "ElOne";

		if (!m_isServer)
		{			
			m_net->sendInfo(p);
			Data dt = m_net->recvData();
			m_hook->setStats(dt);
			m_hook->setpause(pause);
		}
		else
		{
			m_net->recvAllInfo();
			m_net->addInfo(p);
			m_net->broadcastData();
			m_hook->setStats(m_net->getData());
			m_hook->setpause(pause);
		}		

		if (GetAsyncKeyState(VK_PRIOR))
			m_hook->setpSel((m_hook->getpSel() - 1) % m_hook->getStats().players.size()); //select next player
		if (GetAsyncKeyState(VK_NEXT))
			m_hook->setpSel((m_hook->getpSel() + 1) % m_hook->getStats().players.size());	//select previous player
	}
	else
	{
		if (GetAsyncKeyState(VK_F1))
		{
			m_net->create(1338);
			m_isConnect = true;
			m_hook->setIsCon(m_isConnect);
			m_hook->setSolo(false);
			m_isServer = true;
		}
		else if (GetAsyncKeyState(VK_F2))
		{
			char* ip = m_mem.getChar(ipOffsets, 15);
			std::string ipStr = std::string(ip);
			ipStr.resize(15);
			m_net->conn("90.4.94.161", m_mem.GetDouble(portClientOffsets) + 1);
			//m_net->conn("127.0.0.1", 1337);
			m_isConnect = true;
			m_hook->setIsCon(m_isConnect);
			m_hook->setSolo(false);
		}		
	}

	if (GetAsyncKeyState(VK_F3))
	{
		m_hook->setSolo(true);
	}
	m_hook->render();
}

void Mods::Stop()
{
	delete m_net;
	delete m_hook;
}

void Mods::ShowHUD()
{
	GetStats();

	fixHealth();
	fixStat(m_maxHealths, (double&)m_stats.maxHealth);
	fixStat(m_damages, m_stats.damage);
	fixStat(m_regens, m_stats.regeneration);
	fixStat(m_crits, m_stats.critical);
	fixStat(m_attackSpeeds, m_stats.attackSpeed);
	fixStat(m_strengths, m_stats.strength);
	fixStat(m_levels, (double&)m_stats.level);
	fixStat(m_items, (double&)m_stats.item);

	m_hook->setStat(m_stats);
}

void Mods::GetStats()
{
	m_stats.health = m_mem.GetDouble(health_offsets);
	m_stats.maxHealth = m_mem.GetDouble(max_health_offsets);

	m_stats.damage = m_mem.GetDouble(damage_offsets);
	m_stats.attackSpeed = m_mem.GetDouble(attackSpeed_offsets);
	m_stats.strength = m_mem.GetDouble(resistance_offsets);
	m_stats.regeneration = m_mem.GetDouble(regeneration_offsets);
	m_stats.critical = m_mem.GetDouble(crit_offsets);

	m_stats.item = m_mem.GetDouble(item_offsets)+1;
	m_stats.level = m_mem.GetDouble(level_offsets);
}

void Mods::fixHealth()
{
	if (m_stats.health >= 1 && m_stats.health < 100000)
		m_stableHealth = m_stats.health;
	else
		m_stats.health = m_stableHealth;
}

void Mods::fixStat(std::vector<double>& vector, double& stats)
{
	if (vector.size() > 20)
		vector.erase(vector.begin());

	if (stats > 0)
		vector.push_back(stats);

	if (vector.size() > 19)
	{
		m_counts.clear();
		for (int i = 0; i < vector.size(); ++i)
		{
			m_it = m_counts.find(vector[i]);

			if (m_it != m_counts.end())
				m_it->second++;
			else
				m_counts[vector[i]] = 1;
		}

		int max = 0;
		int pos = 0;
		int i = 0;
		for (auto it = m_counts.begin(); it != m_counts.end(); it++)
		{
			if (it->second > max)
			{
				max = it->second;
				pos = i;
			}
			i++;
		}

		m_it = m_counts.begin();
		for (auto j = 0; j < pos; j++)
			m_it++;
		stats = m_it->first;
	}
}